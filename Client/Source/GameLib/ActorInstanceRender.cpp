#include "StdAfx.h"
#include "../EterLib/StateManager.h"

#include "ActorInstance.h"

bool CActorInstance::ms_isDirLine = false;

bool CActorInstance::IsDirLine()
{
    return ms_isDirLine;
}

void CActorInstance::ShowDirectionLine(bool isVisible)
{
    ms_isDirLine = isVisible;
}

void CActorInstance::SetMaterialAlpha(DWORD dwAlpha)
{
    m_dwMtrlAlpha = dwAlpha;
}

void CActorInstance::OnRender()
{
    D3DMATERIAL9 kMtrl;
    STATEMANAGER.GetMaterial(&kMtrl);

    kMtrl.Diffuse = D3DXCOLOR(m_dwMtrlColor);
    STATEMANAGER.SetMaterial(&kMtrl);

    // 현재는 이렇게.. 최종적인 형태는 Diffuse와 Blend의 분리로..
    // 아니면 이런 형태로 가되 Texture & State Sorting 지원으로.. - [levites]
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // --------------------------------------------------------
    // Render intent dispatch (FFP backend)
    // --------------------------------------------------------
    if (m_iColorOp == COLOR_OP_ADD)
    {
        __RenderTintFFP(D3DTOP_ADD);
    }
    else if (m_iColorOp == COLOR_OP_MODULATE)
    {
        __RenderTintFFP(D3DTOP_MODULATE);
    }
    else
    {
        switch (m_iAlphaMode)
        {
        case ALPHA_MODE_MASK:
            __RenderMaskFFP();
            break;

        case ALPHA_MODE_BLEND:
            __RenderBlendFFP();
            break;
        }
    }

    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

    kMtrl.Diffuse = D3DXCOLOR(0xffffffff);
    STATEMANAGER.SetMaterial(&kMtrl);

    if (ms_isDirLine)
    {
        D3DXVECTOR3 kD3DVt3Cur(m_x, m_y, m_z);

        D3DXVECTOR3 kD3DVt3LookDir(0.0f, -1.0f, 0.0f);
        D3DXMATRIX kD3DMatLook;
        D3DXMatrixRotationZ(&kD3DMatLook, D3DXToRadian(GetRotation()));
        D3DXVec3TransformCoord(&kD3DVt3LookDir, &kD3DVt3LookDir, &kD3DMatLook);
        D3DXVec3Scale(&kD3DVt3LookDir, &kD3DVt3LookDir, 200.0f);
        D3DXVec3Add(&kD3DVt3LookDir, &kD3DVt3LookDir, &kD3DVt3Cur);

        D3DXVECTOR3 kD3DVt3AdvDir(0.0f, -1.0f, 0.0f);
        D3DXMATRIX kD3DMatAdv;
        D3DXMatrixRotationZ(&kD3DMatAdv, D3DXToRadian(GetAdvancingRotation()));
        D3DXVec3TransformCoord(&kD3DVt3AdvDir, &kD3DVt3AdvDir, &kD3DMatAdv);
        D3DXVec3Scale(&kD3DVt3AdvDir, &kD3DVt3AdvDir, 200.0f);
        D3DXVec3Add(&kD3DVt3AdvDir, &kD3DVt3AdvDir, &kD3DVt3Cur);

        static CScreen s_kScreen;

        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
        STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);
        STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
        STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

        s_kScreen.SetDiffuseColor(1.0f, 1.0f, 0.0f);
        s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, kD3DVt3Cur.z, kD3DVt3AdvDir.x, kD3DVt3AdvDir.y, kD3DVt3AdvDir.z);

        s_kScreen.SetDiffuseColor(0.0f, 1.0f, 1.0f);
        s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, kD3DVt3Cur.z, kD3DVt3LookDir.x, kD3DVt3LookDir.y, kD3DVt3LookDir.z);

        STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
        STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);

        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
        STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
        STATEMANAGER.RestoreVertexShader();
    }
}

void CActorInstance::__RenderMaskFFP()
{
    auto SetupStage0_ModulateTexDiffuse = [&]()
        {
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        };

    auto DisableStage1 = [&]()
        {
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        };

    auto RenderSubsets = [&](BOOL bAlphaTestEnable)
        {
            STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, bAlphaTestEnable);

            if (bAlphaTestEnable)
            {
                STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0);
                STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
            }

            __RenderLOD(CGrannyModelInstance::MODEL_TEX_ONE, CGrannyModelInstance::MODEL_PASS_OPAQUE);
            __RenderLOD(CGrannyModelInstance::MODEL_TEX_ONE, CGrannyModelInstance::MODEL_PASS_BLEND);

            if (bAlphaTestEnable)
            {
                STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
                STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
            }

            STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
            STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
        };

    SetupStage0_ModulateTexDiffuse();
    DisableStage1();

    // same behavior you had: first without alpha test, then with alpha test
    RenderSubsets(FALSE);
    RenderSubsets(TRUE);
}

void CActorInstance::__RenderBlendFFP()
{
    if (m_fAlphaValue <= 0.0f)
        return;

    // If fully opaque -> treat as MASK path
    if (m_fAlphaValue >= 1.0f)
    {
        __RenderMaskFFP();
        return;
    }

    auto SetupStage0_ModulateTexDiffuse = [&]()
        {
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        };

    auto DisableStage1 = [&]()
        {
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        };

    // Stage 0: tex * diffuse
    SetupStage0_ModulateTexDiffuse();

    // Alpha comes from TFACTOR (global actor alpha)
    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR,
        D3DXCOLOR(1.0f, 1.0f, 1.0f, m_fAlphaValue));

    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    // Ensure stage 1 doesn't leak
    DisableStage1();

    // Enable blending
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    // Disable alpha test in translucent mode
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    // Render both subsets
    __RenderLOD(CGrannyModelInstance::MODEL_TEX_ONE, CGrannyModelInstance::MODEL_PASS_OPAQUE);
    __RenderLOD(CGrannyModelInstance::MODEL_TEX_ONE, CGrannyModelInstance::MODEL_PASS_BLEND);

    // Restore
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
}

void CActorInstance::__RenderTintFFP(D3DTEXTUREOP stage1ColorOp)
{
    auto SetupStage0_ModulateTexDiffuse = [&]()
        {
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
            STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        };

    auto DisableStage1 = [&]()
        {
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        };

    auto RenderSubsets_NoAlphaTest = [&]()
        {
            STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);

            __RenderLOD(CGrannyModelInstance::MODEL_TEX_ONE, CGrannyModelInstance::MODEL_PASS_OPAQUE);
            __RenderLOD(CGrannyModelInstance::MODEL_TEX_ONE, CGrannyModelInstance::MODEL_PASS_BLEND);

            STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
            STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
        };

    // TFACTOR comes from m_AddColor (used for both ADD and MODULATE modes)
    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, m_AddColor);

    // Stage 0: tex * diffuse (color + alpha)
    SetupStage0_ModulateTexDiffuse();

    // Stage 1: CURRENT <op> TFACTOR
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, stage1ColorOp);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    RenderSubsets_NoAlphaTest();

    // Hard disable stage1 so we never leak it
    DisableStage1();
    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
}

void CActorInstance::RestoreRenderMode()
{
    // Restore default state
    m_iAlphaMode = ALPHA_MODE_MASK;
    m_iColorOp = COLOR_OP_NONE;

    // If a fade is currently running, keep the "old state" consistent with defaults
    if (m_kBlendAlpha.m_isBlending)
    {
        m_kBlendAlpha.m_iOldAlphaMode = m_iAlphaMode;
        m_kBlendAlpha.m_iOldColorOp = m_iColorOp;
    }
}

void CActorInstance::SetAlphaMode(int mode)
{
    m_iAlphaMode = mode;

    if (m_iAlphaMode == ALPHA_MODE_MASK)
    {
        // Mask path should behave like fully opaque unless BlendAlpha system is active
        if (!m_kBlendAlpha.m_isBlending)
            m_fAlphaValue = 1.0f;
    }
}

void CActorInstance::SetColorOp(int op)
{
    m_iColorOp = op;

    // Current policy: color ops are handled via RenderColorFactorPass (opaque-style path)
    // So force alpha mode back to MASK if a color op is enabled.
    if (m_iColorOp != COLOR_OP_NONE)
    {
        m_iAlphaMode = ALPHA_MODE_MASK;
        m_fAlphaValue = 1.0f;
    }
}

void CActorInstance::SetAddColor(const D3DXCOLOR & c_rColor)
{
    m_AddColor = c_rColor;
    m_AddColor.a = 1.0f;
}

void CActorInstance::RenderCollisionData()
{
    static CScreen s_Screen;

    if (m_pAttributeInstance)
    {
        for (DWORD col = 0; col < GetCollisionInstanceCount(); ++col)
        {
            CBaseCollisionInstance * pInstance = GetCollisionInstanceData(col);
            pInstance->Render();
        }
    }

    s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);
    TCollisionPointInstanceList::iterator itor;
    s_Screen.SetDiffuseColor(1.0f, (isShow()) ? 1.0f : 0.0f, 0.0f);
    D3DXVECTOR3 center;
    float r;
    GetBoundingSphere(center, r);
    s_Screen.RenderCircle3d(center.x, center.y, center.z, r);

    s_Screen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
    itor = m_DefendingPointInstanceList.begin();

    for (; itor != m_DefendingPointInstanceList.end(); ++itor)
    {
        const TCollisionPointInstance & c_rInstance = *itor;

        for (DWORD i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
        {
            const CDynamicSphereInstance & c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
            s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x,
                                    c_rSphereInstance.v3Position.y,
                                    c_rSphereInstance.v3Position.z,
                                    c_rSphereInstance.fRadius);
        }
    }

    s_Screen.SetDiffuseColor(0.0f, 1.0f, 0.0f);
    itor = m_BodyPointInstanceList.begin();

    for (; itor != m_BodyPointInstanceList.end(); ++itor)
    {
        const TCollisionPointInstance & c_rInstance = *itor;

        for (DWORD i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
        {
            const CDynamicSphereInstance & c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
            s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x,
                                    c_rSphereInstance.v3Position.y,
                                    c_rSphereInstance.v3Position.z,
                                    c_rSphereInstance.fRadius);
        }
    }

    s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);
//	if (m_SplashArea.fDisappearingTime > GetLocalTime())
    {
        auto itor = m_kSplashArea.SphereInstanceVector.begin();

        for (; itor != m_kSplashArea.SphereInstanceVector.end(); ++itor)
        {
            const CDynamicSphereInstance & c_rInstance = *itor;
            s_Screen.RenderCircle3d(c_rInstance.v3Position.x,
                                    c_rInstance.v3Position.y,
                                    c_rInstance.v3Position.z,
                                    c_rInstance.fRadius);
        }
    }
}

void CActorInstance::RenderToShadowMap()
{
    if (m_iAlphaMode == ALPHA_MODE_BLEND)
    {
        if (GetAlphaValue() < 0.5f)
            return;
    }

    CGraphicThingInstance::RenderToShadowMap();

    if (m_pkHorse)
    {
        m_pkHorse->RenderToShadowMap();
    }
}
