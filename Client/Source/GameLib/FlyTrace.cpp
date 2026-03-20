#include "stdafx.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"

#include "FlyingData.h"
#include "FlyTrace.h"
#include "../EterLib/ShaderProvider.h"
#include "../EterLib/GrpDevice.h"

CDynamicPool<CFlyTrace>		CFlyTrace::ms_kPool;

void CFlyTrace::DestroySystem()
{
    ms_kPool.Destroy();
}

CFlyTrace* CFlyTrace::New()
{
    return ms_kPool.Alloc();
}

void CFlyTrace::Delete(CFlyTrace* pkInst)
{
    pkInst->Destroy();
    ms_kPool.Free(pkInst);
}

CFlyTrace::CFlyTrace()
{
    __Initialize();
}

CFlyTrace::~CFlyTrace()
{
    Destroy();
}


void CFlyTrace::__Initialize()
{
    m_bRectShape = false;
    m_dwColor = 0;
    m_fSize = 0.0f;
    m_fTailLength = 0.0f;
}

void CFlyTrace::Destroy()
{
    m_TimePositionDeque.clear();

    __Initialize();
}

void CFlyTrace::UpdateNewPosition(const D3DXVECTOR3 & v3Position)
{
    m_TimePositionDeque.emplace_front(CTimer::Instance().GetCurrentSecond(), v3Position);

    while (!m_TimePositionDeque.empty() && m_TimePositionDeque.back().first + m_fTailLength < CTimer::Instance().GetCurrentSecond())
    {
        m_TimePositionDeque.pop_back();
    }
}

void CFlyTrace::Create(const CFlyingData::TFlyingAttachData & rFlyingAttachData)
{
    m_dwColor = rFlyingAttachData.dwTailColor;
    m_fTailLength = rFlyingAttachData.fTailLength;
    m_fSize = rFlyingAttachData.fTailSize;
    m_bRectShape = rFlyingAttachData.bRectShape;
}


void CFlyTrace::Update()
{

}

//1. 알파를 쓰려면 색깔만 줄수있다.
//2. 텍스쳐를 쓰려면 알파 없다-_-
struct TFlyVertex
{
    D3DXVECTOR3 p;
    DWORD c;
    D3DXVECTOR2 t;
    TFlyVertex() {};

    TFlyVertex(const D3DXVECTOR3& p, DWORD c, const D3DXVECTOR2 & t): p(p), c(c), t(t) {}
};

struct TFlyVertexSet
{
    TFlyVertex v[6];
    TFlyVertexSet(TFlyVertex * pv)
    {
        memcpy(v, pv, sizeof(v));
    }

    bool operator < (const TFlyVertexSet&) const
    {
        return false;
    }

    TFlyVertexSet& operator = (const TFlyVertexSet& rhs)
    {
        memcpy(v, rhs.v, sizeof(v));
        return *this;
    }
};

using TFlyVertexSetVector = std::vector<std::pair<float, TFlyVertexSet >>;

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> FlyTraceSamplers =
    { {
        { 0, ESamplerState::LinearClamp }
    } };

    constexpr PipelineStateDesc FlyTracePipeline =
    {
        ShaderID::FlyTrace,
        EDepthState::EnabledReadOnly,
        EBlendState::Additive,
        ERasterState::CullNone,
        FlyTraceSamplers.data(),
        FlyTraceSamplers.size()
    };
}

void CFlyTrace::Render()
{
    if (m_TimePositionDeque.size() <= 1)
    {
        return;
    }

    IShaderProvider const* sp = GetShaderProvider();
    if (!sp)
    {
        return;
    }

    if (!sp->BindPipelineState(FlyTracePipeline))
    {
        return;
    }

    FlyTraceShaderInputs in{};
    D3DXMATRIX matWorld;
    D3DXMatrixIdentity(&matWorld);

    // world
    std::memcpy(in.vs.world.data(), &matWorld, sizeof(D3DXMATRIX));

    // viewProj
    D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();
    std::memcpy(in.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));

    CGraphicDevice::UploadFlyTraceConstants(in);

    TFlyVertexSetVector VSVector;

    D3DXMATRIX m;
    CScreen s;
    s.UpdateViewMatrix();
    CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

    if (!pCurrentCamera)
    {
        return;
    }

    const D3DXMATRIX & M = pCurrentCamera->GetViewMatrix();
    D3DXMatrixIdentity(&m);
    D3DXVECTOR3 F(pCurrentCamera->GetView());
    m._31 = F.x;
    m._32 = F.y;
    m._33 = F.z;

    Frustum & frustum = s.GetFrustum();

    TTimePositionDeque::iterator it1;
    TTimePositionDeque::iterator it2;
    it2 = it1 = m_TimePositionDeque.begin();
    ++it2;

    for (; it2 != m_TimePositionDeque.end(); ++it2, ++it1)
    {
        const D3DXVECTOR3& rkOld = it1->second;
        const D3DXVECTOR3& rkNew = it2->second;
        D3DXVECTOR3 B = rkNew - rkOld;

        float radius = max(fabs(B.x), max(fabs(B.y), fabs(B.z))) / 2;
        Vector3d c(it1->second.x + B.x * 0.5f,
                   it1->second.y + B.y * 0.5f,
                   it1->second.z + B.z * 0.5f
                  );

        if (frustum.ViewVolumeTest(c, radius) == VS_OUTSIDE)
        {
            continue;
        }

        float rate1 = (1 - (CTimer::Instance().GetCurrentSecond() - it1->first) / m_fTailLength);
        float rate2 = (1 - (CTimer::Instance().GetCurrentSecond() - it2->first) / m_fTailLength);
        float size1 = m_fSize;
        float size2 = m_fSize;

        if (!m_bRectShape)
        {
            size1 *= rate1;
            size2 *= rate2;
        }

        TFlyVertex v[6] =
        {
            TFlyVertex(D3DXVECTOR3(0.0f, size1, 0.0f), m_dwColor, D3DXVECTOR2(0.0f, 0.0f)),
            TFlyVertex(D3DXVECTOR3(-size1, 0.0f, 0.0f), m_dwColor, D3DXVECTOR2(0.0f, 0.5f)),
            TFlyVertex(D3DXVECTOR3(size1, 0.0f, 0.0f), m_dwColor, D3DXVECTOR2(0.5f, 0.0f)),
            TFlyVertex(D3DXVECTOR3(-size2, 0.0f, 0.0f), m_dwColor, D3DXVECTOR2(0.5f, 1.0f)),
            TFlyVertex(D3DXVECTOR3(size2, 0.0f, 0.0f), m_dwColor, D3DXVECTOR2(1.0f, 0.5f)),
            TFlyVertex(D3DXVECTOR3(0.0f, -size2, 0.0f), m_dwColor, D3DXVECTOR2(1.0f, 1.0f)),
        };


        D3DXVECTOR3 E(M._41, M._42, M._43);
        E = pCurrentCamera->GetEye();
        E -= it1->second;

        D3DXVECTOR3 P;
        D3DXVec3Cross(&P, &B, &E);

        D3DXVECTOR3 U;
        D3DXVec3Cross(&U, &F, &P);
        D3DXVec3Normalize(&U, &U);
        D3DXVECTOR3 R;
        D3DXVec3Cross(&R, &F, &U);
        //D3DXMatrixIdentity(&m);
        m._21 = U.x;
        m._22 = U.y;
        m._23 = U.z;
        m._11 = R.x;
        m._12 = R.y;
        m._13 = R.z;
        int i;

        for (i = 0; i < 6; i++)
        {
            D3DXVec3TransformNormal(&v[i].p, &v[i].p, &m);
        }

        for (i = 0; i < 3; i++)
        {
            v[i].p += it1->second;
        }

        for (; i < 6; i++)
        {
            v[i].p += it2->second;
        }

        //for(i=0;i<6;i++)
        //	Tracenf("#%d:%f %f %f", i, v[i].p.x,v[i].p.y,v[i].p.z);

        VSVector.emplace_back(-D3DXVec3Dot(&E, &pCurrentCamera->GetView()), TFlyVertexSet(v));
    }

    std::ranges::sort(VSVector);

    for (auto it = VSVector.begin(); it != VSVector.end(); ++it)
    {
        STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 4, it->second.v, sizeof(TVertex));
    }
}
