#include "StdAfx.h"
#include "SnowEnvironment.h"

#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/ResourceManager.h"
#include "SnowParticle.h"
#include "../EterLib/ShaderProvider.h"
#include "../EterLib/GrpDevice.h"

void CSnowEnvironment::Enable()
{
    if (!m_bSnowEnable)
    {
        Create();
    }

    m_bSnowEnable = TRUE;
}

void CSnowEnvironment::Disable()
{
    m_bSnowEnable = FALSE;
}

void CSnowEnvironment::Update(const D3DXVECTOR3 & c_rv3Pos)
{
    if (!m_bSnowEnable && m_kVct_pkParticleSnow.empty())
        return;

    m_v3Center = c_rv3Pos;
}

void CSnowEnvironment::Deform()
{
    if (!m_bSnowEnable && m_kVct_pkParticleSnow.empty())
        return;

    const D3DXVECTOR3 & c_rv3Pos = m_v3Center;

    static long s_lLastTime = CTimer::Instance().GetCurrentMillisecond();
    long lcurTime = CTimer::Instance().GetCurrentMillisecond();
    float fElapsedTime = float(lcurTime - s_lLastTime) / 1000.0f;
    s_lLastTime = lcurTime;

    CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();

    if (!pCamera)
    {
        return;
    }

    const D3DXVECTOR3 & c_rv3View = pCamera->GetView();

    D3DXVECTOR3 v3ChangedPos = c_rv3View * 3500.0f + c_rv3Pos;
    v3ChangedPos.z = c_rv3Pos.z;

    auto itor = m_kVct_pkParticleSnow.begin();

    for (; itor != m_kVct_pkParticleSnow.end();)
    {
        CSnowParticle * pSnow = *itor;
        pSnow->Update(fElapsedTime, v3ChangedPos);

        if (!pSnow->IsActivate())
        {
            CSnowParticle::Delete(pSnow);

            itor = m_kVct_pkParticleSnow.erase(itor);
        }

        else
        {
            ++itor;
        }
    }

    if (m_bSnowEnable)
    {
        for (int p = 0; p < min(10, m_dwParticleMaxNum - m_kVct_pkParticleSnow.size()); ++p)
        {
            CSnowParticle * pSnowParticle = CSnowParticle::New();
            pSnowParticle->Init(v3ChangedPos);
            m_kVct_pkParticleSnow.push_back(pSnowParticle);
        }
    }
}

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> kSnowSamplers =
    { {
        { 0, ESamplerState::LinearWrap }
    } };

    constexpr PipelineStateDesc kSnowParticlePipeline =
    {
        ShaderID::SnowParticle,
        EDepthState::EnabledReadOnly,
        EBlendState::AlphaBlend,
        ERasterState::CullNone,
        kSnowSamplers.data(),
        kSnowSamplers.size()
    };
}

void CSnowEnvironment::Render()
{
    if (!m_bSnowEnable && m_kVct_pkParticleSnow.empty())
        return;

    DWORD dwParticleCount = min(m_dwParticleMaxNum, m_kVct_pkParticleSnow.size());

    CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();

    if (!pCamera)
    {
        return;
    }

    const D3DXVECTOR3 & c_rv3Up = pCamera->GetUp();
    const D3DXVECTOR3 & c_rv3Cross = pCamera->GetCross();

    SParticleVertex * pv3Verticies;

    if (SUCCEEDED(m_pVB->Lock(0, sizeof(SParticleVertex)*dwParticleCount * 4, (void**) &pv3Verticies, D3DLOCK_DISCARD)))
    {
        int i = 0;
        auto itor = m_kVct_pkParticleSnow.begin();

        for (; i < dwParticleCount && itor != m_kVct_pkParticleSnow.end(); ++i, ++itor)
        {
            CSnowParticle * pSnow = *itor;
            pSnow->SetCameraVertex(c_rv3Up, c_rv3Cross);
            pSnow->GetVerticies(pv3Verticies[i * 4 + 0],
                                pv3Verticies[i * 4 + 1],
                                pv3Verticies[i * 4 + 2],
                                pv3Verticies[i * 4 + 3]);
        }

        m_pVB->Unlock();

        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kSnowParticlePipeline))
            return;

        SnowParticleShaderInputs in{};

        D3DXMATRIX wvp;
        sp->ComputeWorldViewProj(GetIdentityMatrix(), wvp);

        std::memcpy(in.vs.viewProj.data(), &wvp, sizeof(D3DXMATRIX));

        CGraphicDevice::UploadSnowParticleConstants(in);
    }

    m_pImageInstance->GetGraphicImagePointer()->GetTextureReference().SetTextureStage(0);
    STATEMANAGER.SetIndices(m_pIB, 0);
    STATEMANAGER.SetStreamSource(0, m_pVB, sizeof(SParticleVertex));

    STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dwParticleCount * 4, 0, dwParticleCount * 2);
}

bool CSnowEnvironment::__CreateGeometry()
{
    if (FAILED(ms_lpd3dDevice->CreateVertexBuffer(sizeof(SParticleVertex)*m_dwParticleMaxNum * 4,
                                                  D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                                                  NULL,
                                                  D3DPOOL_DEFAULT,
                                                  &m_pVB, nullptr)))
    {
        return false;
    }

    if (FAILED(ms_lpd3dDevice->CreateIndexBuffer(sizeof(WORD)*m_dwParticleMaxNum * 6,
                                                 D3DUSAGE_WRITEONLY,
                                                 D3DFMT_INDEX16,
                                                 D3DPOOL_DEFAULT,
                                                 &m_pIB, nullptr)))
    {
        return false;
    }

    WORD* dstIndices;

    if (FAILED(m_pIB->Lock(0, sizeof(WORD)*m_dwParticleMaxNum * 6, (void**)&dstIndices, 0)))
    {
        return false;
    }

    const WORD c_awFillRectIndices[6] = { 0, 2, 1, 2, 3, 1, };

    for (int i = 0; i < m_dwParticleMaxNum; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            dstIndices[i * 6 + j] = i * 4 + c_awFillRectIndices[j];
        }
    }

    m_pIB->Unlock();
    return true;
}

bool CSnowEnvironment::Create()
{
    Destroy();

    if (!__CreateGeometry())
    {
        return false;
    }

    CGraphicImage * pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer("d:/ymir work/special/snow.dds");
    m_pImageInstance = CGraphicImageInstance::New();
    m_pImageInstance->SetImagePointer(pImage);

    return true;
}

void CSnowEnvironment::Destroy()
{
    SAFE_RELEASE(m_pVB);
    SAFE_RELEASE(m_pIB);

    stl_wipe(m_kVct_pkParticleSnow);
    CSnowParticle::DestroyPool();

    if (m_pImageInstance)
    {
        CGraphicImageInstance::Delete(m_pImageInstance);
        m_pImageInstance = NULL;
    }

    __Initialize();
}

void CSnowEnvironment::__Initialize()
{
    m_bSnowEnable = FALSE;
    m_pVB = NULL;
    m_pIB = NULL;
    m_pImageInstance = NULL;

    m_kVct_pkParticleSnow.reserve(m_dwParticleMaxNum);
}

CSnowEnvironment::CSnowEnvironment()
{
    m_dwParticleMaxNum = 3000;

    __Initialize();
}

CSnowEnvironment::~CSnowEnvironment()
{
    Destroy();
}
