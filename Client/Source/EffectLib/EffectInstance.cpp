#include "StdAfx.h"
#include "EffectInstance.h"
#include "ParticleSystemInstance.h"

#include "../EterBase/Stl.h"
#include "../MilesLib/SoundManager.h"

CDynamicPool<CEffectInstance>	CEffectInstance::ms_kPool;
int CEffectInstance::ms_iRenderingEffectCount = 0;

bool CEffectInstance::LessRenderOrder(CEffectInstance* pkEftInst)
{
    return (m_pkEftData < pkEftInst->m_pkEftData);
}

void CEffectInstance::ResetRenderingEffectCount()
{
    ms_iRenderingEffectCount = 0;
}

int CEffectInstance::GetRenderingEffectCount()
{
    return ms_iRenderingEffectCount;
}

CEffectInstance* CEffectInstance::New()
{
    CEffectInstance* pkEftInst = ms_kPool.Alloc();
    return pkEftInst;
}

void CEffectInstance::Delete(CEffectInstance* pkEftInst)
{
    pkEftInst->Clear();
    ms_kPool.Free(pkEftInst);
}

void CEffectInstance::DestroySystem()
{
    ms_kPool.Destroy();

    CParticleSystemInstance::DestroySystem();
    CEffectMeshInstance::DestroySystem();
}

void CEffectInstance::UpdateSound()
{
    if (m_pSoundInstanceVector)
    {
        CSoundManager& rkSndMgr = CSoundManager::Instance();
        rkSndMgr.UpdateSoundInstance(m_matGlobal._41, m_matGlobal._42, m_matGlobal._43, m_dwFrame, m_pSoundInstanceVector);
        // NOTE : 매트릭스에서 위치를 직접 얻어온다 - [levites]
    }

    ++m_dwFrame;
}

struct FEffectUpdator
{
    BOOL isAlive;
    float fElapsedTime;
    FEffectUpdator(float fElapsedTime)
        : isAlive(FALSE), fElapsedTime(fElapsedTime)
    {
    }

    void operator()(CEffectElementBaseInstance * pInstance)
    {
        if (pInstance->Update(fElapsedTime))
        {
            isAlive = TRUE;
        }
    }
};

void CEffectInstance::OnUpdate()
{
    Transform();

    FEffectUpdator f(CTimer::Instance().GetCurrentSecond() - m_fLastTime);

    f = std::for_each(m_ParticleInstanceVector.begin(), m_ParticleInstanceVector.end(), f);
    f = std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(), f);
    m_isAlive = f.isAlive;

    m_fLastTime = CTimer::Instance().GetCurrentSecond();
}

void CEffectInstance::OnRender()
{
    std::ranges::for_each(m_ParticleInstanceVector, std::mem_fn(&CEffectElementBaseInstance::Render));
    std::ranges::for_each(m_MeshInstanceVector, std::mem_fn(&CEffectElementBaseInstance::Render));

    ++ms_iRenderingEffectCount;
}

void CEffectInstance::SetGlobalMatrix(const D3DXMATRIX & c_rmatGlobal)
{
    m_matGlobal = c_rmatGlobal;
}

BOOL CEffectInstance::isAlive()
{
    return m_isAlive;
}

void CEffectInstance::SetActive()
{
    std::ranges::for_each(m_ParticleInstanceVector, std::mem_fn(&CEffectElementBaseInstance::SetActive));
    std::ranges::for_each(m_MeshInstanceVector, std::mem_fn(&CEffectElementBaseInstance::SetActive));
}

void CEffectInstance::SetDeactive()
{
    std::ranges::for_each(m_ParticleInstanceVector, std::mem_fn(&CEffectElementBaseInstance::SetDeactive));
    std::ranges::for_each(m_MeshInstanceVector, std::mem_fn(&CEffectElementBaseInstance::SetDeactive));
}

void CEffectInstance::__SetParticleData(CParticleSystemData * pData)
{
    CParticleSystemInstance * pInstance = CParticleSystemInstance::New();
    pInstance->SetDataPointer(pData);
    pInstance->SetLocalMatrixPointer(&m_matGlobal);

    m_ParticleInstanceVector.push_back(pInstance);
}

void CEffectInstance::__SetMeshData(CEffectMeshScript * pMesh)
{
    CEffectMeshInstance * pMeshInstance = CEffectMeshInstance::New();
    pMeshInstance->SetDataPointer(pMesh);
    pMeshInstance->SetLocalMatrixPointer(&m_matGlobal);

    m_MeshInstanceVector.push_back(pMeshInstance);
}

void CEffectInstance::SetEffectDataPointer(CEffectData * pEffectData)
{
    m_isAlive = true;

    m_pkEftData = pEffectData;

    m_fLastTime = CTimer::Instance().GetCurrentSecond();
    m_fBoundingSphereRadius = pEffectData->GetBoundingSphereRadius();
    m_v3BoundingSpherePosition = pEffectData->GetBoundingSpherePosition();

    if (m_fBoundingSphereRadius > 0.0f)
    {
        CGraphicObjectInstance::RegisterBoundingSphere();
    }

    DWORD i;

    for (i = 0; i < pEffectData->GetParticleCount(); ++i)
    {
        CParticleSystemData * pParticle = pEffectData->GetParticlePointer(i);

        __SetParticleData(pParticle);
    }

    for (i = 0; i < pEffectData->GetMeshCount(); ++i)
    {
        CEffectMeshScript * pMesh = pEffectData->GetMeshPointer(i);

        __SetMeshData(pMesh);
    }

    m_pSoundInstanceVector = pEffectData->GetSoundInstanceVector();
}

bool CEffectInstance::GetBoundingSphere(D3DXVECTOR3 & v3Center, float& fRadius)
{
    v3Center.x = m_matGlobal._41 + m_v3BoundingSpherePosition.x;
    v3Center.y = m_matGlobal._42 + m_v3BoundingSpherePosition.y;
    v3Center.z = m_matGlobal._43 + m_v3BoundingSpherePosition.z;
    fRadius = m_fBoundingSphereRadius;
    return true;
}

void CEffectInstance::Clear()
{
    if (!m_ParticleInstanceVector.empty())
    {
        std::ranges::for_each(m_ParticleInstanceVector, CParticleSystemInstance::Delete);
        m_ParticleInstanceVector.clear();
    }

    if (!m_MeshInstanceVector.empty())
    {
        std::ranges::for_each(m_MeshInstanceVector, CEffectMeshInstance::Delete);
        m_MeshInstanceVector.clear();
    }

    __Initialize();
}

void CEffectInstance::__Initialize()
{
    m_isAlive = FALSE;
    m_dwFrame = 0;
    m_pSoundInstanceVector = NULL;
    m_fBoundingSphereRadius = 0.0f;
    m_v3BoundingSpherePosition.x = m_v3BoundingSpherePosition.y = m_v3BoundingSpherePosition.z = 0.0f;

    m_pkEftData = NULL;

    D3DXMatrixIdentity(&m_matGlobal);
}

CEffectInstance::CEffectInstance()
{
    __Initialize();
}

CEffectInstance::~CEffectInstance()
{
    assert(m_ParticleInstanceVector.empty());
    assert(m_MeshInstanceVector.empty());
}
