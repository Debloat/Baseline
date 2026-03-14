#include "StdAfx.h"
#include "StateManager.h"

//#define StateManager_Assert(a) if (!(a)) puts("assert"#a)
#define StateManager_Assert(a) assert(a)

CStateManager::CStateManager(LPDIRECT3DDEVICE9EX lpDevice) : m_lpD3DDev(nullptr)
{
    m_bScene = false;
    SetDevice(lpDevice);
}

CStateManager::~CStateManager()
{
    if (m_lpD3DDev)
    {
        m_lpD3DDev->Release();
        m_lpD3DDev = nullptr;
    }
}

void CStateManager::SetDefaultState()
{
    m_CurrentState.ResetState();
    m_CopyState.ResetState();

    m_bScene = false;
    m_bForce = true;

    D3DXMATRIX Identity;
    D3DXMatrixIdentity(&Identity);

    SetTransform(D3DTS_WORLD, &Identity);
    SetTransform(D3DTS_VIEW, &Identity);
    SetTransform(D3DTS_PROJECTION, &Identity);

    SetRenderState(D3DRS_ALPHAREF, 1);
    SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
    SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
    SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
    SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
    SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
    SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    SetRenderState(D3DRS_ZENABLE, TRUE);
    SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    SetRenderState(D3DRS_STENCILENABLE, FALSE);
    SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    SetRenderState(D3DRS_CLIPPING, TRUE);
    SetRenderState(D3DRS_WRAP0, 0);
    SetRenderState(D3DRS_WRAP1, 0);
    SetRenderState(D3DRS_WRAP2, 0);
    SetRenderState(D3DRS_WRAP3, 0);
    SetRenderState(D3DRS_WRAP4, 0);
    SetRenderState(D3DRS_WRAP5, 0);
    SetRenderState(D3DRS_WRAP6, 0);
    SetRenderState(D3DRS_WRAP7, 0);

    for (DWORD i = 0; i < 8; ++i)
    {
        SetTexture(i, nullptr);
    }

    SetPixelShader(nullptr);

    D3DXVECTOR4 av4Null[STATEMANAGER_MAX_VCONSTANTS];
    memset(av4Null, 0, sizeof(av4Null));
    SetVertexShaderConstant(0, av4Null, STATEMANAGER_MAX_VCONSTANTS);
    SetPixelShaderConstant(0, av4Null, STATEMANAGER_MAX_PCONSTANTS);

    m_bForce = false;

#ifdef _DEBUG

    for (int i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
    {
        m_bRenderStateSavingFlag[i] = false;
    }

    for (int j = 0; j < STATEMANAGER_MAX_TRANSFORMSTATES; j++)
    {
        m_bTransformSavingFlag[j] = false;
    }

    for (int j = 0; j < STATEMANAGER_MAX_STAGES; ++j)
    {
        for (int i = 0; i < STATEMANAGER_MAX_TEXTURESTATES; ++i)
        {
            m_bTextureStageStateSavingFlag[j][i] = false;
        }

        for (int i = 0; i < STATEMANAGER_MAX_SAMPLERSTATES; ++i)
        {
            m_bSamplerStateSavingFlag[j][i] = false;
        }
    }

#endif _DEBUG
}

void CStateManager::Restore()
{
    m_bForce = true;

    for (int i = 0; i < STATEMANAGER_MAX_RENDERSTATES; ++i)
    {
        SetRenderState(D3DRENDERSTATETYPE(i), m_CurrentState.m_RenderStates[i]);
    }

    for (int i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
        for (int j = 0; j < STATEMANAGER_MAX_TEXTURESTATES; ++j)
        {
            SetTextureStageState(i, D3DTEXTURESTAGESTATETYPE(j), m_CurrentState.m_TextureStates[i][j]);
        }

    for (int i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
        for (int j = 0; j < STATEMANAGER_MAX_SAMPLERSTATES; ++j)
        {
            SetSamplerState(i, D3DSAMPLERSTATETYPE(j), m_CurrentState.m_SamplerStates[i][j]);
        }

    for (int i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
    {
        SetTexture(i, m_CurrentState.m_Textures[i]);
    }

    m_bForce = false;
}

bool CStateManager::BeginScene()
{
    m_bScene = true;

    D3DXMATRIX m4Proj;
    D3DXMATRIX m4View;
    D3DXMATRIX m4World;
    GetTransform(D3DTS_WORLD, &m4World);
    GetTransform(D3DTS_PROJECTION, &m4Proj);
    GetTransform(D3DTS_VIEW, &m4View);
    SetTransform(D3DTS_WORLD, &m4World);
    SetTransform(D3DTS_PROJECTION, &m4Proj);
    SetTransform(D3DTS_VIEW, &m4View);

    if (FAILED(m_lpD3DDev->BeginScene()))
    {
        return false;
    }

    return true;
}

void CStateManager::EndScene()
{
    m_lpD3DDev->EndScene();
    m_bScene = false;
}

// Material
void CStateManager::SaveMaterial()
{
    m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
}

void CStateManager::SaveMaterial(const D3DMATERIAL9* pMaterial)
{
    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
    SetMaterial(pMaterial);
}

void CStateManager::RestoreMaterial()
{
    SetMaterial(&m_CopyState.m_D3DMaterial);
}

void CStateManager::SetMaterial(const D3DMATERIAL9* pMaterial)
{
    m_lpD3DDev->SetMaterial(pMaterial);
    m_CurrentState.m_D3DMaterial = *pMaterial;
}

void CStateManager::GetMaterial(D3DMATERIAL9* pMaterial)
{
    // Set the renderstate and remember it.
    *pMaterial = m_CurrentState.m_D3DMaterial;
}

// Renderstates
DWORD CStateManager::GetRenderState(D3DRENDERSTATETYPE Type)
{
    return m_CurrentState.m_RenderStates[Type];
}

void CStateManager::SaveRenderState(D3DRENDERSTATETYPE Type, DWORD dwValue)
{
#ifdef _DEBUG

    if (m_bRenderStateSavingFlag[Type])
    {
        Tracef(" CStateManager::SaveRenderState - This render state is already saved [%d, %d]\n", Type, dwValue);
        StateManager_Assert(!" This render state is already saved!");
    }

    m_bRenderStateSavingFlag[Type] = TRUE;
#endif _DEBUG

    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_RenderStates[Type] = m_CurrentState.m_RenderStates[Type];
    SetRenderState(Type, dwValue);
}

void CStateManager::RestoreRenderState(D3DRENDERSTATETYPE Type)
{
#ifdef _DEBUG

    if (!m_bRenderStateSavingFlag[Type])
    {
        Tracef(" CStateManager::SaveRenderState - This render state was not saved [%d, %d]\n", Type);
        StateManager_Assert(!" This render state was not saved!");
    }

    m_bRenderStateSavingFlag[Type] = FALSE;
#endif _DEBUG

    SetRenderState(Type, m_CopyState.m_RenderStates[Type]);
}

void CStateManager::SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value)
{
    if (m_CurrentState.m_RenderStates[Type] == Value)
    {
        return;
    }

    m_lpD3DDev->SetRenderState(Type, Value);
    m_CurrentState.m_RenderStates[Type] = Value;
}

void CStateManager::GetRenderState(D3DRENDERSTATETYPE Type, DWORD* pdwValue)
{
    *pdwValue = m_CurrentState.m_RenderStates[Type];
}

// Textures
void CStateManager::SaveTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture)
{
    // Check that we have set this up before, if not, the default is this.
    m_CopyState.m_Textures[dwStage] = m_CurrentState.m_Textures[dwStage];
    SetTexture(dwStage, pTexture);
}

void CStateManager::RestoreTexture(DWORD dwStage)
{
    SetTexture(dwStage, m_CopyState.m_Textures[dwStage]);
}

void CStateManager::SetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture)
{
    if (pTexture == m_CurrentState.m_Textures[dwStage])
    {
        return;
    }

    m_lpD3DDev->SetTexture(dwStage, pTexture);
    m_CurrentState.m_Textures[dwStage] = pTexture;
}

void CStateManager::SetVertexTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture)
{
    const DWORD vsStage = D3DVERTEXTEXTURESAMPLER0 + dwStage;

    if (pTexture == m_CurrentState.m_Textures[vsStage])
    {
        return;
    }

    m_lpD3DDev->SetTexture(vsStage, pTexture);
    m_CurrentState.m_Textures[vsStage] = pTexture;
}


void CStateManager::GetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9* ppTexture)
{
    *ppTexture = m_CurrentState.m_Textures[dwStage];
}

// Texture stage states
void CStateManager::SaveTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
    // Check that we have set this up before, if not, the default is this.
#ifdef _DEBUG
    if (m_bTextureStageStateSavingFlag[dwStage][Type])
    {
        Tracef(" CStateManager::SaveTextureStageState - This texture stage state is already saved [%d, %d]\n", dwStage, Type);
        StateManager_Assert(!" This texture stage state is already saved!");
    }

    m_bTextureStageStateSavingFlag[dwStage][Type] = TRUE;
#endif _DEBUG
    m_CopyState.m_TextureStates[dwStage][Type] = m_CurrentState.m_TextureStates[dwStage][Type];
    SetTextureStageState(dwStage, Type, dwValue);
}

void CStateManager::RestoreTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type)
{
#ifdef _DEBUG

    if (!m_bTextureStageStateSavingFlag[dwStage][Type])
    {
        Tracef(" CStateManager::RestoreTextureStageState - This texture stage state was not saved [%d, %d]\n", dwStage, Type);
        StateManager_Assert(!" This texture stage state was not saved!");
    }

    m_bTextureStageStateSavingFlag[dwStage][Type] = FALSE;
#endif _DEBUG
    SetTextureStageState(dwStage, Type, m_CopyState.m_TextureStates[dwStage][Type]);
}

void CStateManager::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
    if (m_CurrentState.m_TextureStates[dwStage][Type] == dwValue)
    {
        return;
    }

    m_lpD3DDev->SetTextureStageState(dwStage, Type, dwValue);
    m_CurrentState.m_TextureStates[dwStage][Type] = dwValue;
}

// Sampler states
void CStateManager::SaveSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
#ifdef _DEBUG

    if (m_bSamplerStateSavingFlag[dwStage][Type])
    {
        Tracef(" CStateManager::SaveTextureStageState - This texture stage state is already saved [%d, %d]\n", dwStage, Type);
        StateManager_Assert(!" This texture stage state is already saved!");
    }

    m_bSamplerStateSavingFlag[dwStage][Type] = TRUE;
#endif _DEBUG
    m_CopyState.m_SamplerStates[dwStage][Type] = m_CurrentState.m_SamplerStates[dwStage][Type];
    SetSamplerState(dwStage, Type, dwValue);
}

void CStateManager::RestoreSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type)
{
#ifdef _DEBUG

    if (!m_bSamplerStateSavingFlag[dwStage][Type])
    {
        Tracef(" CStateManager::RestoreTextureStageState - This texture stage state was not saved [%d, %d]\n", dwStage, Type);
        StateManager_Assert(!" This texture stage state was not saved!");
    }

    m_bSamplerStateSavingFlag[dwStage][Type] = FALSE;
#endif _DEBUG
    SetSamplerState(dwStage, Type, m_CopyState.m_SamplerStates[dwStage][Type]);
}

void CStateManager::SetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
    if (m_CurrentState.m_SamplerStates[dwStage][Type] == dwValue)
    {
        return;
    }

    m_lpD3DDev->SetSamplerState(dwStage, Type, dwValue);
    m_CurrentState.m_SamplerStates[dwStage][Type] = dwValue;
}

// Vertex Shader
void CStateManager::RestoreVertexShader()
{
    SetVertexShader(m_CopyState.m_dwVertexShader);
}

void CStateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader)
{
    m_lpD3DDev->SetVertexShader(dwShader);
    m_CurrentState.m_dwVertexShader = dwShader;
}

void CStateManager::GetVertexShader(LPDIRECT3DVERTEXSHADER9* pdwShader)
{
    *pdwShader = m_CurrentState.m_dwVertexShader;
}

// Vertex Declaration
void CStateManager::SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader)
{
    m_lpD3DDev->SetVertexDeclaration(dwShader);
    m_CurrentState.m_dwVertexDeclaration = dwShader;
}

// FVF
void CStateManager::SaveFVF(DWORD dwShader)
{
    m_CopyState.m_dwFVF = m_CurrentState.m_dwFVF;
    SetFVF(dwShader);
}

void CStateManager::RestoreFVF()
{
    SetFVF(m_CopyState.m_dwFVF);
}

void CStateManager::SetFVF(DWORD dwShader)
{
    m_lpD3DDev->SetFVF(dwShader);
    m_CurrentState.m_dwFVF = dwShader;
}

void CStateManager::GetFVF(DWORD* pdwShader)
{
    *pdwShader = m_CurrentState.m_dwFVF;
}

// Pixel Shader
void CStateManager::SetPixelShader(LPDIRECT3DPIXELSHADER9 dwShader)
{
    if (m_CurrentState.m_dwPixelShader == dwShader)
    {
        return;
    }

    m_lpD3DDev->SetPixelShader(dwShader);
    m_CurrentState.m_dwPixelShader = dwShader;
}

void CStateManager::GetPixelShader(LPDIRECT3DPIXELSHADER9* pdwShader)
{
    *pdwShader = m_CurrentState.m_dwPixelShader;
}

// *** These states are cached, but not protected from multiple sends of the same value.
// Transform
void CStateManager::SaveTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)
{
#ifdef _DEBUG

    if (m_bTransformSavingFlag[Type])
    {
        Tracef(" CStateManager::SaveTransform - This transform is already saved [%d]\n", Type);
        StateManager_Assert(!" This trasform is already saved!");
    }

    m_bTransformSavingFlag[Type] = TRUE;
#endif _DEBUG

    m_CopyState.m_Matrices[Type] = m_CurrentState.m_Matrices[Type];
    SetTransform(Type, (D3DXMATRIX*)pMatrix);
}

void CStateManager::RestoreTransform(D3DTRANSFORMSTATETYPE Type)
{
#ifdef _DEBUG

    if (!m_bTransformSavingFlag[Type])
    {
        Tracef(" CStateManager::RestoreTransform - This transform was not saved [%d]\n", Type);
        StateManager_Assert(!" This render state was not saved!");
    }

    m_bTransformSavingFlag[Type] = FALSE;
#endif _DEBUG

    SetTransform(Type, &m_CopyState.m_Matrices[Type]);
}

// Don't cache-check the transform.  To much to do
void CStateManager::SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)
{
    if (m_bScene)
    {
        m_lpD3DDev->SetTransform(Type, pMatrix);
    }

    else
    {
        assert(D3DTS_VIEW == Type || D3DTS_PROJECTION == Type || D3DTS_WORLD == Type);
    }

    m_CurrentState.m_Matrices[Type] = *pMatrix;
}

void CStateManager::GetTransform(D3DTRANSFORMSTATETYPE Type, D3DMATRIX* pMatrix)
{
    *pMatrix = m_CurrentState.m_Matrices[Type];
}

// SetVertexShaderConstant
void CStateManager::SetVertexShaderConstant(DWORD dwRegister, CONST void* pConstantData, DWORD dwConstantCount)
{
    m_lpD3DDev->SetVertexShaderConstantF(dwRegister, (const float*)pConstantData, dwConstantCount);

    // Set the renderstate and remember it.
    for (DWORD i = 0; i < dwConstantCount; i++)
    {
        StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
        m_CurrentState.m_VertexShaderConstants[dwRegister + i] = *(((D3DXVECTOR4*)pConstantData) + i);
    }
}

// SetPixelShaderConstant
void CStateManager::SetPixelShaderConstant(DWORD dwRegister, CONST void* pConstantData, DWORD dwConstantCount)
{
    m_lpD3DDev->SetPixelShaderConstantF(dwRegister, *(D3DXVECTOR4*)pConstantData, dwConstantCount);

    // Set the renderstate and remember it.
    for (DWORD i = 0; i < dwConstantCount; i++)
    {
        StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
        m_CurrentState.m_PixelShaderConstants[dwRegister + i] = *(((D3DXVECTOR4*)pConstantData) + i);
    }
}

void CStateManager::SetStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, UINT Stride)
{
    CStreamData kStreamData(pStreamData, Stride);

    if (m_CurrentState.m_StreamData[StreamNumber] == kStreamData)
    {
        return;
    }

    m_lpD3DDev->SetStreamSource(StreamNumber, pStreamData, 0, Stride);
    m_CurrentState.m_StreamData[StreamNumber] = kStreamData;
}

void CStateManager::SaveIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, UINT BaseVertexIndex)
{
    m_CopyState.m_IndexData = m_CurrentState.m_IndexData;
    SetIndices(pIndexData, BaseVertexIndex);
}

void CStateManager::SetIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, UINT BaseVertexIndex)
{
    CIndexData kIndexData(pIndexData, BaseVertexIndex);

    if (m_CurrentState.m_IndexData == kIndexData)
    {
        return;
    }

    m_lpD3DDev->SetIndices(pIndexData);
    m_CurrentState.m_IndexData = kIndexData;
}

HRESULT CStateManager::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
    return (m_lpD3DDev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
}

HRESULT CStateManager::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    m_CurrentState.m_StreamData[0] = nullptr;
    return (m_lpD3DDev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride));
}

HRESULT CStateManager::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount, INT baseVertexIndex)
{
    return (m_lpD3DDev->DrawIndexedPrimitive(PrimitiveType, baseVertexIndex, minIndex, NumVertices, startIndex, primCount));
}

HRESULT CStateManager::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData,
                                              D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
    m_CurrentState.m_IndexData = nullptr;
    m_CurrentState.m_StreamData[0] = nullptr;
    return (m_lpD3DDev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,
                                               VertexStreamZeroStride));
}

LPDIRECT3DDEVICE9EX CStateManager::GetDevice()
{
    return m_lpD3DDev;
}

void CStateManager::SetDevice(LPDIRECT3DDEVICE9EX lpDevice)
{
    StateManager_Assert(lpDevice);
    lpDevice->AddRef();

    if (m_lpD3DDev)
    {
        m_lpD3DDev->Release();
        m_lpD3DDev = nullptr;
    }

    m_lpD3DDev = lpDevice;

    SetDefaultState();
}
