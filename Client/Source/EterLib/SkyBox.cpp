#include "StdAfx.h"
#include "SkyBox.h"
#include "Camera.h"
#include "StateManager.h"
#include "ResourceManager.h"

/* - SHADER -------------------------------------------- */
#include "ShaderVertexDeclarations.h"
#include "GrpDevice.h"
/* ----------------------------------------------------- */

//////////////////////////////////////////////////////////////////////////
// CSkyObjectQuad
//////////////////////////////////////////////////////////////////////////

CSkyObjectQuad::CSkyObjectQuad()
{
    // Index buffer
    m_Indices[0] = 0;
    m_Indices[1] = 2;
    m_Indices[2] = 1;
    m_Indices[3] = 3;

    for (auto& v : m_Vertex)
    {
        v = TPDTVertex{};
    }
}

CSkyObjectQuad::~CSkyObjectQuad()
{
}

void CSkyObjectQuad::SetVertex(const unsigned char& c_rucNumVertex, const TPDTVertex & c_rPDTVertex)
{
    if (c_rucNumVertex > 3)
    {
        TraceError("CSkyObjectQuad::SetVertex: Invalid vertex number %d\n", c_rucNumVertex);
        return;
    }

    memcpy(&m_Vertex[m_Indices[c_rucNumVertex]], &c_rPDTVertex, sizeof(TPDTVertex));
}

void CSkyObjectQuad::Render()
{
    if (CGraphicBase::SetPDTStream(m_Vertex, 4))
    {
        STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
}

//////////////////////////////////////////////////////////////////////////
// CSkyObject
/////////////////////////////////////////////////////////////////////////
CSkyObject::CSkyObject() :
    m_v3Position(0.0f, 0.0f, 0.0f),
    m_fScaleX(1.0f),
    m_fScaleY(1.0f),
    m_fScaleZ(1.0f)
{
    D3DXMatrixIdentity(&m_matWorld);
    D3DXMatrixIdentity(&m_matTranslation);
    D3DXMatrixIdentity(&m_matTextureCloud);

    m_bSkyMatrixUpdated = false;
}

CSkyObject::~CSkyObject()
{
    Destroy();
}

void CSkyObject::Destroy()
{
}

void CSkyObject::Update()
{
    D3DXVECTOR3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();

    if (m_v3Position == v3Eye)
        if (m_bSkyMatrixUpdated == false)
        {
            return;
        }

    m_v3Position = v3Eye;

    m_matWorld._41 = m_v3Position.x;
    m_matWorld._42 = m_v3Position.y;
    m_matWorld._43 = m_v3Position.z;

    m_matWorldCloud._41 = m_v3Position.x;
    m_matWorldCloud._42 = m_v3Position.y;
    m_matWorldCloud._43 = m_v3Position.z + m_fCloudHeight;

    if (m_bSkyMatrixUpdated)
    {
        m_bSkyMatrixUpdated = false;
    }
}

void CSkyObject::Render()
{
}

CGraphicImageInstance* CSkyObject::GenerateTexture(const char* szfilename)
{
    assert(szfilename != NULL);

    if (strlen(szfilename) <= 0)
    {
        assert(false);
    }

    CResource * pResource = CResourceManager::Instance().GetResourcePointer(szfilename);

    if (!pResource->IsType(CGraphicImage::Type()))
    {
        assert(false);
        return NULL;
    }

    CGraphicImageInstance * pImageInstance = CGraphicImageInstance::New();
    pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
    return (pImageInstance);
}

void CSkyObject::DeleteTexture(CGraphicImageInstance * pImageInstance)
{
    if (pImageInstance)
    {
        CGraphicImageInstance::Delete(pImageInstance);
    }
}

//////////////////////////////////////////////////////////////////////////
// CSkyObject::TSkyObjectFace
//////////////////////////////////////////////////////////////////////////

void CSkyObject::TSkyObjectFace::Render()
{
    for (unsigned char uci = 0; uci < m_SkyObjectQuadVector.size(); ++uci)
    {
        m_SkyObjectQuadVector[uci].Render();
    }
}

//////////////////////////////////////////////////////////////////////////
// CSkyBox
//////////////////////////////////////////////////////////////////////////

CSkyBox::CSkyBox()
{
}

CSkyBox::~CSkyBox()
{
    Destroy();
}

void CSkyBox::Update()
{
    CSkyObject::Update();
}

void CSkyBox::Destroy()
{
    Unload();
}

void CSkyBox::SetSkyBoxScale(const D3DXVECTOR3 & c_rv3Scale)
{
    m_fScaleX = c_rv3Scale.x;
    m_fScaleY = c_rv3Scale.y;
    m_fScaleZ = c_rv3Scale.z;

    m_bSkyMatrixUpdated = true;
    D3DXMatrixScaling(&m_matWorld, m_fScaleX, m_fScaleY, m_fScaleZ);
}

void CSkyBox::SetTexture(const char* c_szFileName)
{
    if (!c_szFileName || !*c_szFileName)
    {
        TraceError("CSkyBox::SetTexture: empty filename");
        return;
    }

    m_strSkyTextureFileName = c_szFileName;

    if (auto itor = m_GraphicImageInstanceMap.find(m_strSkyTextureFileName); itor != m_GraphicImageInstanceMap.end())
    {
        return;
    }

    CGraphicImageInstance* pGraphicImageInstance = GenerateTexture(m_strSkyTextureFileName.c_str());
    m_GraphicImageInstanceMap.try_emplace(m_strSkyTextureFileName, pGraphicImageInstance);
}

void CSkyBox::SetCloudTexture(const char* c_szFileName)
{
    if (auto itor = m_GraphicImageInstanceMap.find(c_szFileName); m_GraphicImageInstanceMap.end() != itor)
    {
        return;
    }

    m_FaceCloud.m_strfacename = c_szFileName;
    CGraphicImageInstance * pGraphicImageInstance = GenerateTexture(c_szFileName);
    m_GraphicImageInstanceMap.try_emplace(m_FaceCloud.m_strfacename, pGraphicImageInstance);
}

void CSkyBox::SetCloudScale(const D3DXVECTOR2 & c_rv2CloudScale)
{
    m_fCloudScaleX = c_rv2CloudScale.x;
    m_fCloudScaleY = c_rv2CloudScale.y;

    D3DXMatrixScaling(&m_matWorldCloud, m_fCloudScaleX, m_fCloudScaleY, 1.0f);
}

void CSkyBox::SetCloudHeight(float fHeight)
{
    m_fCloudHeight = fHeight;
}

void CSkyBox::SetCloudTextureScale(const D3DXVECTOR2 & c_rv2CloudTextureScale)
{
    m_fCloudTextureScaleX = c_rv2CloudTextureScale.x;
    m_fCloudTextureScaleY = c_rv2CloudTextureScale.y;

    m_matTextureCloud._11 = m_fCloudTextureScaleX;
    m_matTextureCloud._22 = m_fCloudTextureScaleY;
}

void CSkyBox::SetCloudScrollSpeed(const D3DXVECTOR2 & c_rv2CloudScrollSpeed)
{
    m_fCloudScrollSpeedU = c_rv2CloudScrollSpeed.x;
    m_fCloudScrollSpeedV = c_rv2CloudScrollSpeed.y;
}

void CSkyBox::SetCloudTintMultiplier(const D3DXCOLOR& c_rvCloudTintMultiplier)
{
    m_cloudTintMultiplier = c_rvCloudTintMultiplier;
}

CGraphicImageInstance* CSkyBox::GetSkyTextureInstance() const
{
    if (m_strSkyTextureFileName.empty())
        return nullptr;

    auto it = m_GraphicImageInstanceMap.find(m_strSkyTextureFileName);
    if (it == m_GraphicImageInstanceMap.end())
        return nullptr;

    return it->second;
}

void CSkyBox::Unload()
{
    auto itor = m_GraphicImageInstanceMap.begin();

    while (itor != m_GraphicImageInstanceMap.end())
    {
        DeleteTexture(itor->second);
        ++itor;
    }

    m_GraphicImageInstanceMap.clear();
}

void CSkyBox::SetSkyObjectQuadHorizon(
    TSkyObjectQuadVector* pSkyObjectQuadVector,
    const D3DXVECTOR3* c_pv3QuadPoints,
    float uMin, float uMax,
    float vMin, float vMax)
{
    pSkyObjectQuadVector->clear();
    pSkyObjectQuadVector->resize(1);

    CSkyObjectQuad& rSkyObjectQuad = (*pSkyObjectQuadVector)[0];

    // Quad winding is:
    // 0,2,1,3 (triangle strip)
    // Keep same vertex ordering but assign proper atlas UVs.

    const float u[4] = { uMin, uMin, uMax, uMax };
    const float v[4] = { vMax, vMin, vMax, vMin };

    TPDTVertex aPDTVertex{};

    for (unsigned char i = 0; i < 4; ++i)
    {
        aPDTVertex.position = c_pv3QuadPoints[i];
        aPDTVertex.texCoord.x = u[i];
        aPDTVertex.texCoord.y = v[i];

        rSkyObjectQuad.SetVertex(i, aPDTVertex);
    }
}

struct SkyFaceDesc
{
    const char* name;
    D3DXVECTOR3 points[4];
};

void CSkyBox::Refresh()
{
    const float tileW = 1.0f / 4.0f;
    const float tileH = 1.0f / 3.0f;

    D3DXVECTOR3 v3QuadPoints[4];

    static const SkyFaceDesc faces[6] =
    {
        { "front", {
            {  1.0f, -1.0f, -1.0f },
            {  1.0f, -1.0f,  1.0f },
            { -1.0f, -1.0f, -1.0f },
            { -1.0f, -1.0f,  1.0f }
        }},
        { "back", {
            { -1.0f,  1.0f, -1.0f },
            { -1.0f,  1.0f,  1.0f },
            {  1.0f,  1.0f, -1.0f },
            {  1.0f,  1.0f,  1.0f }
        }},
        { "left", {
            {  1.0f,  1.0f, -1.0f },
            {  1.0f,  1.0f,  1.0f },
            {  1.0f, -1.0f, -1.0f },
            {  1.0f, -1.0f,  1.0f }
        }},
        { "right", {
            { -1.0f, -1.0f, -1.0f },
            { -1.0f, -1.0f,  1.0f },
            { -1.0f,  1.0f, -1.0f },
            { -1.0f,  1.0f,  1.0f }
        }},
        { "top", {
            {  1.0f, -1.0f,  1.0f },
            {  1.0f,  1.0f,  1.0f },
            { -1.0f, -1.0f,  1.0f },
            { -1.0f,  1.0f,  1.0f }
        }},
        { "bottom", {
            {  1.0f, -1.0f, -1.0f },
            {  1.0f,  1.0f, -1.0f },
            { -1.0f, -1.0f, -1.0f },
            { -1.0f,  1.0f, -1.0f }
        }},
    };

    // Face order in your engine:
    // 0 front
    // 1 back
    // 2 left
    // 3 right
    // 4 top
    // 5 bottom

    // Unity cross mapping:
    //        [ ][+Y][ ][ ]
    // [-X][+Z][+X][-Z]
    //        [ ][-Y][ ][ ]

    // FRONT  = +Z = col 1, row 1
    SetSkyObjectQuadHorizon(&m_Faces[0].m_SkyObjectQuadVector, faces[0].points,
        tileW * 1, tileW * 2,
        tileH * 1, tileH * 2);

    // BACK   = -Z = col 3, row 1
    SetSkyObjectQuadHorizon(&m_Faces[1].m_SkyObjectQuadVector, faces[1].points,
        tileW * 3, tileW * 4,
        tileH * 1, tileH * 2);

    // LEFT   = -X = col 0, row 1
    SetSkyObjectQuadHorizon(&m_Faces[2].m_SkyObjectQuadVector, faces[2].points,
        tileW * 0, tileW * 1,
        tileH * 1, tileH * 2);

    // RIGHT  = +X = col 2, row 1
    SetSkyObjectQuadHorizon(&m_Faces[3].m_SkyObjectQuadVector, faces[3].points,
        tileW * 2, tileW * 3,
        tileH * 1, tileH * 2);

    // TOP    = +Y = col 1, row 0
    SetSkyObjectQuadHorizon(&m_Faces[4].m_SkyObjectQuadVector, faces[4].points,
        tileW * 1, tileW * 2,
        tileH * 0, tileH * 1);

    // BOTTOM = -Y = col 1, row 2
    SetSkyObjectQuadHorizon(&m_Faces[5].m_SkyObjectQuadVector, faces[5].points,
        tileW * 1, tileW * 2,
        tileH * 2, tileH * 3);

    //// Clouds..
    v3QuadPoints[0] = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
    v3QuadPoints[1] = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
    v3QuadPoints[2] = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
    v3QuadPoints[3] = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
    SetSkyObjectQuadHorizon(&m_FaceCloud.m_SkyObjectQuadVector, v3QuadPoints,
        0.0f, 1.0f,
        0.0f, 1.0f);
}

void CSkyBox::Render()
{
    // 2004.01.25 myevan Ã³¸®¸¦ ·»´õ¸µ ÈÄ¹ÝÀ¸·Î ¿Å±â°í, DepthTest Ã³¸®
    STATEMANAGER.SaveRenderState(D3DRS_ZENABLE,	TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    /* - SHADER [SKYBOX] ----------------------------------- */
    IShaderProvider const* sp = GetShaderProvider();
    if (!sp || !sp->BindShader(ShaderID::SkyBox))
    {
        TraceError("Skybox shader bind failed");
        return;
    }

    STATEMANAGER.SetVertexDeclaration(CShaderInputLayouts::Get(EShaderInputLayout::PCT));

    /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */
    SkyboxShaderInputs in{};
    D3DXMATRIX matWVP;
    sp->ComputeWorldViewProj(m_matWorld, matWVP);
    std::memcpy(in.vs.worldViewProj.data(), &matWVP, sizeof(D3DXMATRIX));

    CGraphicDevice::UploadSkyboxConstants(in);
    /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

    /* ----------------------------------------------------- */

    //Render Face
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU,	D3DTADDRESS_CLAMP);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV,	D3DTADDRESS_CLAMP);

    if (m_strSkyTextureFileName.empty())
    {
        TraceError("CSkyBox::Render: Sky texture filename not set");
        assert(false);
        return;
    }

    auto it = m_GraphicImageInstanceMap.find(m_strSkyTextureFileName);
    if (it == m_GraphicImageInstanceMap.end() || !it->second)
    {
        TraceError("CSkyBox::Render: Sky texture instance missing (%s)", m_strSkyTextureFileName.c_str());
        assert(false);
        return;
    }

    CGraphicImageInstance* pSkyImageInstance = it->second;

    STATEMANAGER.SetTexture(0, pSkyImageInstance->GetTextureReference().GetD3DTexture());

    for (unsigned int i = 0; i < 6; ++i)
    {
        m_Faces[i].Render();
    }

    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);

    /* - SHADER [SKYBOX] ----------------------------------- */
    STATEMANAGER.SetTexture(0, nullptr);
    STATEMANAGER.SetVertexShader(nullptr);
    STATEMANAGER.SetPixelShader(nullptr);
    STATEMANAGER.SetVertexDeclaration(nullptr);
    /* ----------------------------------------------------- */

    STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

void CSkyBox::RenderCloud()
{
    CGraphicImageInstance * pCloudGraphicImageInstance = m_GraphicImageInstanceMap[m_FaceCloud.m_strfacename];

    if (!pCloudGraphicImageInstance)
    {
        return;
    }

    // 2004.01.25 myevan Ã³¸®¸¦ ·»´õ¸µ ÈÄ¹ÝÀ¸·Î ¿Å±â°í, DepthTest Ã³¸®
    STATEMANAGER.SaveRenderState(D3DRS_ZENABLE,	TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

    /* - SHADER [CLOUDS] ----------------------------------- */
    IShaderProvider const* sp = GetShaderProvider();
    const FrameShaderInputs& frame = sp->GetFrameShaderInputs();

    if (!sp || !sp->BindShader(ShaderID::Cloud))
    {
        TraceError("Cloud shader bind failed");
        return;
    }

    STATEMANAGER.SetVertexDeclaration(CShaderInputLayouts::Get(EShaderInputLayout::PCT));

    /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */
    D3DXMATRIX world = m_matWorldCloud;

    D3DXMATRIX matWVP;
    sp->ComputeWorldViewProj(world, matWVP);

    CloudShaderInputs in{};
    std::memcpy(in.vs.worldViewProj.data(), &matWVP, sizeof(D3DXMATRIX));

    in.vs.uvScaleSpeed = { m_fCloudTextureScaleX, m_fCloudTextureScaleY, m_fCloudScrollSpeedU, m_fCloudScrollSpeedV };
    in.vs.timeSeconds[0] = frame.timeSeconds;

    in.ps.cloudTint = { m_cloudTintMultiplier.r, m_cloudTintMultiplier.g, m_cloudTintMultiplier.b, m_cloudTintMultiplier.a };

    CGraphicDevice::UploadCloudConstants(in);
    /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

    /* ----------------------------------------------------- */

    STATEMANAGER.SetTexture(0, pCloudGraphicImageInstance->GetTexturePointer()->GetD3DTexture());
    m_FaceCloud.Render();

    /* - SHADER [CLOUDS] ----------------------------------- */
    STATEMANAGER.SetTexture(0, nullptr);
    STATEMANAGER.SetVertexShader(nullptr);
    STATEMANAGER.SetPixelShader(nullptr);
    STATEMANAGER.SetVertexDeclaration(nullptr);
    /* ----------------------------------------------------- */

    STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}
