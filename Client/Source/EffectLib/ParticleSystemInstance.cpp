#include "StdAfx.h"
#include "../EterBase/Random.h"
#include "../EterLib/StateManager.h"
#include "ParticleSystemData.h"
#include "ParticleSystemInstance.h"
#include "ParticleInstance.h"

#include "../EterLib/ShaderProvider.h"
#include "../EterLib/GrpDevice.h"

CDynamicPool<CParticleSystemInstance>	CParticleSystemInstance::ms_kPool;

using namespace NEffectUpdateDecorator;

void CParticleSystemInstance::DestroySystem()
{
    ms_kPool.Destroy();

    CParticleInstance::DestroySystem();
    //CRayParticleInstance::DestroySystem();
}

CParticleSystemInstance* CParticleSystemInstance::New()
{
    return ms_kPool.Alloc();
}

void CParticleSystemInstance::Delete(CParticleSystemInstance* pkPSInst)
{
    pkPSInst->Destroy();
    ms_kPool.Free(pkPSInst);
}


DWORD CParticleSystemInstance::GetEmissionCount()
{
    return m_dwCurrentEmissionCount;
}

void CParticleSystemInstance::CreateParticles(float fElapsedTime)
{
    float fEmissionCount;
    m_pEmitterProperty->GetEmissionCountPerSecond(m_fLocalTime, &fEmissionCount);

    float fCreatingValue = fEmissionCount * (fElapsedTime / 1.0f) + m_fEmissionResidue;
    int iCreatingCount = int(fCreatingValue);
    m_fEmissionResidue = fCreatingValue - iCreatingCount;

    int icurEmissionCount = GetEmissionCount();
    int iMaxEmissionCount = int(m_pEmitterProperty->GetMaxEmissionCount());
    int iNextEmissionCount = int(icurEmissionCount + iCreatingCount);
    iCreatingCount -= max(0, iNextEmissionCount - iMaxEmissionCount);

    float fLifeTime = 0.0f;
    float fEmittingSize = 0.0f;
    D3DXVECTOR3 _v3TimePosition;
    D3DXVECTOR3 _v3Velocity;
    float fVelocity = 0.0f;
    D3DXVECTOR2 v2HalfSize;
    float fLieRotation = 0;

    if (iCreatingCount)
    {
        m_pEmitterProperty->GetParticleLifeTime(m_fLocalTime, &fLifeTime);

        if (fLifeTime == 0.0f)
        {
            return;
        }

        m_pEmitterProperty->GetEmittingSize(m_fLocalTime, &fEmittingSize);

        m_pData->GetPosition(m_fLocalTime, _v3TimePosition);

        m_pEmitterProperty->GetEmittingDirectionX(m_fLocalTime, &_v3Velocity.x);
        m_pEmitterProperty->GetEmittingDirectionY(m_fLocalTime, &_v3Velocity.y);
        m_pEmitterProperty->GetEmittingDirectionZ(m_fLocalTime, &_v3Velocity.z);

        m_pEmitterProperty->GetEmittingVelocity(m_fLocalTime, &fVelocity);

        m_pEmitterProperty->GetParticleSizeX(m_fLocalTime, &v2HalfSize.x);
        m_pEmitterProperty->GetParticleSizeY(m_fLocalTime, &v2HalfSize.y);

        if (BILLBOARD_TYPE_LIE == m_pParticleProperty->m_byBillboardType && mc_pmatLocal)
        {
            float fsx = mc_pmatLocal->_32;
            float fcx = sqrtf(1.0f - fsx * fsx);

            if (fcx >= 0.00001f)
            {
                fLieRotation = D3DXToDegree(atan2f(-mc_pmatLocal->_12, mc_pmatLocal->_22));
            }
        }

    }

    CParticleInstance * pFirstInstance = 0;

    for (int i = 0; i < iCreatingCount; ++i)
    {
        CParticleInstance * pInstance;

        pInstance = CParticleInstance::New();
        pInstance->m_pParticleProperty = m_pParticleProperty;
        pInstance->m_pEmitterProperty = m_pEmitterProperty;

        // LifeTime
        pInstance->m_fLifeTime = fLifeTime;
        pInstance->m_fLastLifeTime = fLifeTime;

        // Position
        switch (m_pEmitterProperty->GetEmitterShape())
        {
            case CEmitterProperty::EMITTER_SHAPE_POINT:
                pInstance->m_v3Position.x = 0.0f;
                pInstance->m_v3Position.y = 0.0f;
                pInstance->m_v3Position.z = 0.0f;
                break;

            case CEmitterProperty::EMITTER_SHAPE_ELLIPSE:
                pInstance->m_v3Position.x = frandom(-500.0f, 500.0f);
                pInstance->m_v3Position.y = frandom(-500.0f, 500.0f);
                pInstance->m_v3Position.z = 0.0f;
                D3DXVec3Normalize(&pInstance->m_v3Position, &pInstance->m_v3Position);

                if (m_pEmitterProperty->isEmitFromEdge())
                {
                    pInstance->m_v3Position *= (m_pEmitterProperty->m_fEmittingRadius + fEmittingSize);
                }

                else
                {
                    pInstance->m_v3Position *= (frandom(0.0f, m_pEmitterProperty->m_fEmittingRadius) + fEmittingSize);
                }

                break;

            case CEmitterProperty::EMITTER_SHAPE_SQUARE:
                pInstance->m_v3Position.x = (frandom(-m_pEmitterProperty->m_v3EmittingSize.x / 2.0f, m_pEmitterProperty->m_v3EmittingSize.x / 2.0f) + fEmittingSize);
                pInstance->m_v3Position.y = (frandom(-m_pEmitterProperty->m_v3EmittingSize.y / 2.0f, m_pEmitterProperty->m_v3EmittingSize.y / 2.0f) + fEmittingSize);
                pInstance->m_v3Position.z = (frandom(-m_pEmitterProperty->m_v3EmittingSize.z / 2.0f, m_pEmitterProperty->m_v3EmittingSize.z / 2.0f) + fEmittingSize);
                break;

            case CEmitterProperty::EMITTER_SHAPE_SPHERE:
                pInstance->m_v3Position.x = frandom(-500.0f, 500.0f);
                pInstance->m_v3Position.y = frandom(-500.0f, 500.0f);
                pInstance->m_v3Position.z = frandom(-500.0f, 500.0f);
                D3DXVec3Normalize(&pInstance->m_v3Position, &pInstance->m_v3Position);

                if (m_pEmitterProperty->isEmitFromEdge())
                {
                    pInstance->m_v3Position *= (m_pEmitterProperty->m_fEmittingRadius + fEmittingSize);
                }

                else
                {
                    pInstance->m_v3Position *= (frandom(0.0f, m_pEmitterProperty->m_fEmittingRadius) + fEmittingSize);
                }

                break;
        }

        // Position
        D3DXVECTOR3 v3TimePosition = _v3TimePosition;

        pInstance->m_v3Position += v3TimePosition;

        if (mc_pmatLocal && !m_pParticleProperty->m_bAttachFlag)
        {
            D3DXVec3TransformCoord(&pInstance->m_v3Position, &pInstance->m_v3Position, mc_pmatLocal);
            D3DXVec3TransformCoord(&v3TimePosition, &v3TimePosition, mc_pmatLocal);
        }

        pInstance->m_v3StartPosition = v3TimePosition;
        // NOTE : Update를 호출하지 않고 Rendering 되기 때문에 length가 0이 되는 문제가 있다.
        //        Velocity를 구한 후 그만큼 빼준 값으로 초기화 해주도록 바꿨음 - [levites]
        //pInstance->m_v3LastPosition = pInstance->m_v3Position;

        // Direction & Velocity
        pInstance->m_v3Velocity.x = 0.0f;
        pInstance->m_v3Velocity.y = 0.0f;
        pInstance->m_v3Velocity.z = 0.0f;

        if (CEmitterProperty::EMITTER_ADVANCED_TYPE_INNER == m_pEmitterProperty->GetEmitterAdvancedType())
        {
            auto d3dd = (pInstance->m_v3Position - v3TimePosition);
            D3DXVec3Normalize(&pInstance->m_v3Velocity, &d3dd);
            pInstance->m_v3Velocity *= -100.0f;
        }

        else if (CEmitterProperty::EMITTER_ADVANCED_TYPE_OUTER == m_pEmitterProperty->GetEmitterAdvancedType())
        {
            if (m_pEmitterProperty->GetEmitterShape() == CEmitterProperty::EMITTER_SHAPE_POINT)
            {
                pInstance->m_v3Velocity.x = frandom(-100.0f, 100.0f);
                pInstance->m_v3Velocity.y = frandom(-100.0f, 100.0f);
                pInstance->m_v3Velocity.z = frandom(-100.0f, 100.0f);
            }

            else
            {
                auto d3dd = (pInstance->m_v3Position - v3TimePosition);
                D3DXVec3Normalize(&pInstance->m_v3Velocity, &d3dd);
                pInstance->m_v3Velocity *= 100.0f;
            }
        }

        D3DXVECTOR3 v3Velocity = _v3Velocity;

        if (mc_pmatLocal && !m_pParticleProperty->m_bAttachFlag)
        {
            D3DXVec3TransformNormal(&v3Velocity, &v3Velocity, mc_pmatLocal);
        }

        pInstance->m_v3Velocity += v3Velocity;

        if (m_pEmitterProperty->m_v3EmittingDirection.x > 0.0f)
        {
            pInstance->m_v3Velocity.x += frandom(-m_pEmitterProperty->m_v3EmittingDirection.x / 2.0f, m_pEmitterProperty->m_v3EmittingDirection.x / 2.0f) * 1000.0f;
        }

        if (m_pEmitterProperty->m_v3EmittingDirection.y > 0.0f)
        {
            pInstance->m_v3Velocity.y += frandom(-m_pEmitterProperty->m_v3EmittingDirection.y / 2.0f, m_pEmitterProperty->m_v3EmittingDirection.y / 2.0f) * 1000.0f;
        }

        if (m_pEmitterProperty->m_v3EmittingDirection.z > 0.0f)
        {
            pInstance->m_v3Velocity.z += frandom(-m_pEmitterProperty->m_v3EmittingDirection.z / 2.0f, m_pEmitterProperty->m_v3EmittingDirection.z / 2.0f) * 1000.0f;
        }

        pInstance->m_v3Velocity *= fVelocity;

        // Size
        pInstance->m_v2HalfSize = v2HalfSize;

        // Rotation
        pInstance->m_fRotation = m_pParticleProperty->m_wRotationRandomStartingBegin;
        pInstance->m_fRotation = frandom(m_pParticleProperty->m_wRotationRandomStartingBegin, m_pParticleProperty->m_wRotationRandomStartingEnd);

        // Rotation - Lie 일 경우 LocalMatrix 의 Rotation 값을 Random 에 적용한다.
        //            매번 할 필요는 없을듯. 어느 정도의 최적화가 필요. - [levites]
        if (BILLBOARD_TYPE_LIE == m_pParticleProperty->m_byBillboardType && mc_pmatLocal)
        {
            pInstance->m_fRotation += fLieRotation;
        }

        // Texture Animation
        pInstance->m_byFrameIndex = 0;
        pInstance->m_byTextureAnimationType = m_pParticleProperty->GetTextureAnimationType();

        if (m_pParticleProperty->GetTextureAnimationFrameCount() > 1)
        {
            if (CParticleProperty::TEXTURE_ANIMATION_TYPE_RANDOM_DIRECTION == m_pParticleProperty->GetTextureAnimationType())
            {
                if (random() & 1)
                {
                    pInstance->m_byFrameIndex = 0;
                    pInstance->m_byTextureAnimationType = CParticleProperty::TEXTURE_ANIMATION_TYPE_CW;
                }

                else
                {
                    pInstance->m_byFrameIndex = m_pParticleProperty->GetTextureAnimationFrameCount() - 1;
                    pInstance->m_byTextureAnimationType = CParticleProperty::TEXTURE_ANIMATION_TYPE_CCW;
                }
            }

            if (m_pParticleProperty->m_bTexAniRandomStartFrameFlag)
            {
                pInstance->m_byFrameIndex = random_range(0, m_pParticleProperty->GetTextureAnimationFrameCount() - 1);
            }
        }

        // Simple Update
        {
            pInstance->m_v3LastPosition = pInstance->m_v3Position - (pInstance->m_v3Velocity * fElapsedTime);
            pInstance->m_v2Scale.x = m_pParticleProperty->m_TimeEventScaleX.front().m_Value;
            pInstance->m_v2Scale.y = m_pParticleProperty->m_TimeEventScaleY.front().m_Value;
            pInstance->m_dcColor = m_pParticleProperty->m_TimeEventColor.front().m_Value;
        }

        if (!pFirstInstance)
        {
            m_pData->BuildDecorator(pInstance);
            pFirstInstance = pInstance;
        }

        else
        {
            pInstance->m_pDecorator = pFirstInstance->m_pDecorator->Clone(pFirstInstance, pInstance);
        }

        m_ParticleInstanceListVector[pInstance->m_byFrameIndex].push_back(pInstance);
        m_dwCurrentEmissionCount++;
    }
}

bool CParticleSystemInstance::OnUpdate(float fElapsedTime)
{
    bool bMakeParticle = true;

    /////

    if (m_fLocalTime >= m_pEmitterProperty->GetCycleLength())
    {
        if (m_pEmitterProperty->isCycleLoop() && --m_iLoopCount != 0)
        {
            if (m_iLoopCount < 0)
            {
                m_iLoopCount = 0;
            }

            m_fLocalTime = m_fLocalTime - m_pEmitterProperty->GetCycleLength();
        }

        else
        {
            bMakeParticle = false;
            m_iLoopCount = 1;

            if (GetEmissionCount() == 0)
            {
                return false;
            }
        }
    }

    /////

    int dwFrameIndex;
    int dwFrameCount = m_pParticleProperty->GetTextureAnimationFrameCount();

    float fAngularVelocity;
    m_pEmitterProperty->GetEmittingAngularVelocity(m_fLocalTime, &fAngularVelocity);

    if (fAngularVelocity && !m_pParticleProperty->m_bAttachFlag)
    {
        auto d3dd = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
        D3DXVec3TransformNormal(&m_pParticleProperty->m_v3ZAxis, &d3dd, mc_pmatLocal);
    }

    for (dwFrameIndex = 0; dwFrameIndex < dwFrameCount; dwFrameIndex++)
    {
        auto itor = m_ParticleInstanceListVector[dwFrameIndex].begin();

        for (; itor != m_ParticleInstanceListVector[dwFrameIndex].end();)
        {
            CParticleInstance * pInstance = *itor;

            if (!pInstance->Update(fElapsedTime, fAngularVelocity))
            {
                pInstance->DeleteThis();

                itor = m_ParticleInstanceListVector[dwFrameIndex].erase(itor);
                m_dwCurrentEmissionCount--;
            }

            else
            {
                if (pInstance->m_byFrameIndex != dwFrameIndex)
                {
                    m_ParticleInstanceListVector[dwFrameCount + pInstance->m_byFrameIndex].push_back(*itor);
                    itor = m_ParticleInstanceListVector[dwFrameIndex].erase(itor);
                }

                else
                {
                    ++itor;
                }
            }
        }
    }

    if (isActive() && bMakeParticle)
    {
        CreateParticles(fElapsedTime);
    }

    for (dwFrameIndex = 0; dwFrameIndex < dwFrameCount; ++dwFrameIndex)
    {
        m_ParticleInstanceListVector[dwFrameIndex].splice(m_ParticleInstanceListVector[dwFrameIndex].end(), m_ParticleInstanceListVector[dwFrameIndex + dwFrameCount]);
        m_ParticleInstanceListVector[dwFrameIndex + dwFrameCount].clear();
    }

    return true;
}

namespace NParticleRenderer
{
    struct TwoSideRenderer
    {
        const D3DXMATRIX* pmat;
        TwoSideRenderer(const D3DXMATRIX * pmat = NULL)
            : pmat(pmat)
        {
        }

        inline void operator()(CParticleInstance * pInstance)
        {
            pInstance->Transform(pmat, D3DXToRadian(-30.0f));
            STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));

            pInstance->Transform(pmat, D3DXToRadian(+30.0f));
            STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
        }
    };

    struct ThreeSideRenderer
    {
        const D3DXMATRIX* pmat;
        ThreeSideRenderer(const D3DXMATRIX * pmat = NULL)
            : pmat(pmat)
        {
        }

        inline void operator()(CParticleInstance * pInstance)
        {
            pInstance->Transform(pmat);
            STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
            pInstance->Transform(pmat, D3DXToRadian(-60.0f));
            STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
            pInstance->Transform(pmat, D3DXToRadian(+60.0f));
            STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
        }
    };

    struct NormalRenderer
    {
        inline void operator()(CParticleInstance * pInstance)
        {
            pInstance->Transform();
            STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
        }
    };

    struct AttachRenderer
    {
        const D3DXMATRIX* pmat;
        AttachRenderer(const D3DXMATRIX * pmat)
            : pmat(pmat)
        {
        }

        inline void operator()(CParticleInstance * pInstance)
        {
            pInstance->Transform(pmat);
            STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
        }
    };
}

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> kEffectParticleSamplers =
    { {
        { 0, ESamplerState::LinearWrap },
    } };

    constexpr PipelineStateDesc kEffectParticlePipeline =
    {
        ShaderID::EffectParticle,
        EDepthState::EnabledReadOnly,
        EBlendState::AlphaBlend,   // overridden per-system
        ERasterState::CullNone,
        kEffectParticleSamplers.data(),
        kEffectParticleSamplers.size()
    };

    inline float MapParticleColorOpToId(BYTE byType)
    {
        // Keep this mapping stable with effect_particle_ps.hlsl.
        switch (byType)
        {
        case D3DTOP_MODULATE:   return 0.0f;
        case D3DTOP_ADD:        return 1.0f;
        case D3DTOP_SUBTRACT:   return 2.0f;
        case D3DTOP_MODULATE2X: return 3.0f;
        case D3DTOP_MODULATE4X: return 4.0f;
        case D3DTOP_ADDSMOOTH:  return 5.0f;
        default:                return 0.0f;
        }
    }

    inline void BuildParticleTextureFactor(DWORD color, std::array<float, 4>& outRGBA)
    {
        const float a = ((color >> 24) & 0xFF) / 255.0f;
        const float r = ((color >> 16) & 0xFF) / 255.0f;
        const float g = ((color >> 8) & 0xFF) / 255.0f;
        const float b = ((color >> 0) & 0xFF) / 255.0f;

        outRGBA = { r, g, b, a };
    }
}

void CParticleSystemInstance::OnRender()
{
    IShaderProvider const* sp = GetShaderProvider();
    if (!sp || !sp->BindPipelineState(kEffectParticlePipeline))
    {
        return;
    }

    // --- Blend (preserve original src/dest behavior) ---
    const int src = m_pParticleProperty->m_bySrcBlendType;
    const int dst = m_pParticleProperty->m_byDestBlendType;

    if (src == D3DBLEND_SRCALPHA && dst == D3DBLEND_INVSRCALPHA)
    {
        sp->BindBlendState(EBlendState::AlphaBlend);
    }
    else if (src == D3DBLEND_SRCALPHA && dst == D3DBLEND_ONE)
    {
        sp->BindBlendState(EBlendState::AlphaAdditive);
    }
    else if (src == D3DBLEND_ONE && dst == D3DBLEND_ONE)
    {
        sp->BindBlendState(EBlendState::Additive);
    }
    else if (src == D3DBLEND_ONE && dst == D3DBLEND_INVSRCCOLOR)
    {
        sp->BindBlendState(EBlendState::One_InvSrcColor);
    }
    else
    {
        TraceError("CParticleSystemInstance::OnRender - No compatible blend state found.");
    }

    // --- Build base inputs once (viewProj + op id) ---
    EffectParticleShaderInputs base{};
    {
        const D3DXMATRIX& view = CGraphicBase::GetViewMatrix();
        const D3DXMATRIX& proj = CGraphicBase::GetProjMatrix();
        D3DXMATRIX viewProj = view * proj;
        std::memcpy(base.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));
    }

    base.ps.ops = { MapParticleColorOpToId(m_pParticleProperty->m_byColorOperationType), 0.0f, 0.0f, 0.0f };

    // --- Render functors that upload per-particle textureFactor ---
    if (m_pParticleProperty->m_byBillboardType < BILLBOARD_TYPE_2FACE)
    {
        if (!m_pParticleProperty->m_bAttachFlag)
        {
            struct Renderer
            {
                EffectParticleShaderInputs* pBase;
                void operator()(CParticleInstance* pInstance)
                {
                    pInstance->Transform();

                    BuildParticleTextureFactor(pInstance->m_dcColor.m_dwColor, pBase->ps.textureFactor);
                    CGraphicDevice::UploadEffectParticleConstants(*pBase);

                    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
                }
            } obj{ &base };

            ForEachParticleRendering(obj);
        }
        else
        {
            struct Renderer
            {
                const D3DXMATRIX* pmat;
                EffectParticleShaderInputs* pBase;

                void operator()(CParticleInstance* pInstance)
                {
                    pInstance->Transform(pmat);

                    BuildParticleTextureFactor(pInstance->m_dcColor.m_dwColor, pBase->ps.textureFactor);
                    CGraphicDevice::UploadEffectParticleConstants(*pBase);

                    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
                }
            } obj{ mc_pmatLocal, &base };

            ForEachParticleRendering(obj);
        }
    }
    else if (m_pParticleProperty->m_byBillboardType == BILLBOARD_TYPE_2FACE)
    {
        struct Renderer
        {
            const D3DXMATRIX* pmat;
            EffectParticleShaderInputs* pBase;

            void operator()(CParticleInstance* pInstance)
            {
                {
                    pInstance->Transform(pmat, D3DXToRadian(-30.0f));

                    BuildParticleTextureFactor(pInstance->m_dcColor.m_dwColor, pBase->ps.textureFactor);
                    CGraphicDevice::UploadEffectParticleConstants(*pBase);

                    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
                }

                {
                    pInstance->Transform(pmat, D3DXToRadian(+30.0f));

                    BuildParticleTextureFactor(pInstance->m_dcColor.m_dwColor, pBase->ps.textureFactor);
                    CGraphicDevice::UploadEffectParticleConstants(*pBase);

                    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
                }
            }
        };

        if (!m_pParticleProperty->m_bAttachFlag)
        {
            Renderer obj{ nullptr, &base };
            ForEachParticleRendering(obj);
        }
        else
        {
            Renderer obj{ mc_pmatLocal, &base };
            ForEachParticleRendering(obj);
        }
    }
    else if (m_pParticleProperty->m_byBillboardType == BILLBOARD_TYPE_3FACE)
    {
        struct Renderer
        {
            const D3DXMATRIX* pmat;
            EffectParticleShaderInputs* pBase;

            void operator()(CParticleInstance* pInstance)
            {
                {
                    pInstance->Transform(pmat);

                    BuildParticleTextureFactor(pInstance->m_dcColor.m_dwColor, pBase->ps.textureFactor);
                    CGraphicDevice::UploadEffectParticleConstants(*pBase);

                    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
                }

                {
                    pInstance->Transform(pmat, D3DXToRadian(-60.0f));

                    BuildParticleTextureFactor(pInstance->m_dcColor.m_dwColor, pBase->ps.textureFactor);
                    CGraphicDevice::UploadEffectParticleConstants(*pBase);

                    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
                }

                {
                    pInstance->Transform(pmat, D3DXToRadian(+60.0f));

                    BuildParticleTextureFactor(pInstance->m_dcColor.m_dwColor, pBase->ps.textureFactor);
                    CGraphicDevice::UploadEffectParticleConstants(*pBase);

                    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pInstance->GetParticleMeshPointer(), sizeof(TPTVertex));
                }
            }
        };

        if (!m_pParticleProperty->m_bAttachFlag)
        {
            Renderer obj{ nullptr, &base };
            ForEachParticleRendering(obj);
        }
        else
        {
            Renderer obj{ mc_pmatLocal, &base };
            ForEachParticleRendering(obj);
        }
    }
}

void CParticleSystemInstance::OnSetDataPointer(CEffectElementBase * pElement)
{
    m_pData = (CParticleSystemData*)pElement;

    m_dwCurrentEmissionCount = 0;
    m_pParticleProperty = m_pData->GetParticlePropertyPointer();
    m_pEmitterProperty = m_pData->GetEmitterPropertyPointer();
    m_iLoopCount = m_pEmitterProperty->GetLoopCount();
    m_ParticleInstanceListVector.resize(m_pParticleProperty->GetTextureAnimationFrameCount() * 2 + 2);

    /////

    assert(m_kVct_pkImgInst.empty());
    m_kVct_pkImgInst.reserve(m_pParticleProperty->m_ImageVector.size());

    for (DWORD i = 0; i < m_pParticleProperty->m_ImageVector.size(); ++i)
    {
        CGraphicImage * pImage = m_pParticleProperty->m_ImageVector[i];

        CGraphicImageInstance* pkImgInstNew = CGraphicImageInstance::New();
        pkImgInstNew->SetImagePointer(pImage);
        m_kVct_pkImgInst.push_back(pkImgInstNew);
    }
}

void CParticleSystemInstance::OnInitialize()
{
    m_dwCurrentEmissionCount = 0;
    m_iLoopCount = 0;
    m_fEmissionResidue = 0.0f;
}

void CParticleSystemInstance::OnDestroy()
{
    // 2004. 3. 1. myevan. 파티클 제거 루틴
    for (auto i = m_ParticleInstanceListVector.begin(); i != m_ParticleInstanceListVector.end(); ++i)
    {
        TParticleInstanceList& rkLst_kParticleInst = *i;

        for (auto j = rkLst_kParticleInst.begin(); j != rkLst_kParticleInst.end(); ++j)
        {
            CParticleInstance* pkParticleInst = *j;
            pkParticleInst->DeleteThis();
        }

        rkLst_kParticleInst.clear();
    }

    m_ParticleInstanceListVector.clear();

    std::ranges::for_each(m_kVct_pkImgInst, CGraphicImageInstance::Delete);
    m_kVct_pkImgInst.clear();
}

CParticleSystemInstance::CParticleSystemInstance()
{
    Initialize();
}

CParticleSystemInstance::~CParticleSystemInstance()
{
    assert(m_ParticleInstanceListVector.empty());
    assert(m_kVct_pkImgInst.empty());
}
