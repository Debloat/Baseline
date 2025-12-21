#include "StdAfx.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/ResourceManager.h"

#include "MapOutdoor.h"
#include "TerrainPatch.h"

/* - SHADER [WATER] ------------------------------------ */
#include "../EterLib/ShaderVertexDeclarations.h"
#include "../EterLib/Camera.h"
#include "../EterLib/GrpDevice.h"
/* ----------------------------------------------------- */
/* - YOSUN_CONTROL_CENTER [Water Wireframe] ------------ */
#include "../SphereLib/YosunControlCenter.h"
/* ----------------------------------------------------- */

void CMapOutdoor::LoadWaterTexture()
{
    UnloadWaterTexture();

    /* - SHADER [WATER] ------------------------------------ */
    auto& wss = GetWaterShaderSettings();

    wss.basic.normalMapImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(wss.basic.normalMapTexturePath);
    m_WaterNormalMapInstance.SetImagePointer(wss.basic.normalMapImage);

    wss.displacement.heightMapImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(wss.displacement.heightMapTexturePath);
    m_WaterHeightMapInstance.SetImagePointer(wss.displacement.heightMapImage);
    /* ----------------------------------------------------- */
}

void CMapOutdoor::UnloadWaterTexture()
{
    /* - SHADER [WATER] ------------------------------------ */
    m_WaterNormalMapInstance.Destroy();
    m_WaterHeightMapInstance.Destroy();
    /* ----------------------------------------------------- */
}
#include "../EterLib/SkyBox.h"
void CMapOutdoor::RenderWater()
{
    if (m_PatchVector.empty())
    {
        return;
    }

    if (!IsVisiblePart(PART_WATER))
    {
        return;
    }

    //////////////////////////////////////////////////////////////////////////
    // RenderState
    D3DXMATRIX matTexTransformWater;

    STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    STATEMANAGER.SaveRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);

    CGraphicImageInstance* skyInst = m_SkyBox.GetSkyTextureInstance();
    if (!skyInst)
    {
        TraceError("RenderWater: Sky texture instance missing (sky reflections need it)");
        assert(false);
        return;
    }

    /* - SHADER [WATER] ------------------------------------ */
    STATEMANAGER.SetTexture(0, skyInst->GetTextureReference().GetD3DTexture());
    STATEMANAGER.SetTexture(1, m_WaterNormalMapInstance.GetTexturePointer()->GetD3DTexture());
    STATEMANAGER.SetVertexTexture(2, m_WaterHeightMapInstance.GetTexturePointer()->GetD3DTexture());
    /* ----------------------------------------------------- */

    D3DXMatrixScaling(&matTexTransformWater, m_fWaterTexCoordBase, -m_fWaterTexCoordBase, 0.0f);
    D3DXMatrixMultiply(&matTexTransformWater, &m_matViewInverse, &matTexTransformWater);

    // Sky atlas must clamp, otherwise cross edges will wrap and seam
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    // Normal map must wrap
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

    // Heightmap uses point sampling
    STATEMANAGER.SaveSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    STATEMANAGER.SaveSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    STATEMANAGER.SaveSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SaveSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    /* - YOSUN_CONTROL_CENTER [Water Wireframe] ------------ */
    const bool wireframe = GetYosunControlSettings().worldEditor.terrain.drawWaterWireFrame;

    if (wireframe)
    {
        STATEMANAGER.SaveRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    }
    /* ----------------------------------------------------- */

    /* - SHADER [WATER] ------------------------------------ */
    IShaderProvider const* sp = GetShaderProvider();
    if (!sp || !sp->BindShader(ShaderID::Water))
    {
        TraceError("Water shader bind failed");
        return;
    }

    STATEMANAGER.SetVertexDeclaration(CShaderInputLayouts::Get(EShaderInputLayout::PTC));
    /* ----------------------------------------------------- */

    // RenderState
    //////////////////////////////////////////////////////////////////////////
    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;
    m_matWorldForCommonUse._43 = 0.0f;

    /* - SHADER [WATER] ------------------------------------ */
    const auto& ws = GetWaterShaderSettings();

    /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */
    const FrameShaderInputs& frame = sp->GetFrameShaderInputs();
    // --- WVP ---
    D3DXMATRIX matWVP;
    sp->ComputeWorldViewProj(m_matWorldForCommonUse, matWVP);
    // --- View ---
    D3DXMATRIX matView;
    std::memcpy(&matView, frame.view.data(), sizeof(D3DXMATRIX));

    WaterShaderInputs in{};
    std::memcpy(in.worldViewProj.data(), &matWVP, sizeof(D3DXMATRIX));
    std::memcpy(in.view.data(), &matView, sizeof(D3DXMATRIX));
    std::memcpy(in.texTransform.data(), &matTexTransformWater, sizeof(D3DXMATRIX));

    in.cameraPos     = frame.cameraPos;
    in.lightDir      = frame.sunDir;
    in.lightColor    = frame.sunColor;
    in.timeSeconds   = frame.timeSeconds;
    in.windDirection = frame.windDirection;
    in.windStrength  = frame.windStrength;

    in.settings = &ws;
    CGraphicDevice::UploadWaterConstants(in);
    /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

    /* ----------------------------------------------------- */

    for (const auto& it : m_PatchVector)
    {
        DrawWater(it.second);
    }

    //////////////////////////////////////////////////////////////////////////
    // RenderState
    for (int i = 0; i < 3; ++i)
    {
        STATEMANAGER.RestoreSamplerState(i, D3DSAMP_MINFILTER);
        STATEMANAGER.RestoreSamplerState(i, D3DSAMP_MAGFILTER);
        STATEMANAGER.RestoreSamplerState(i, D3DSAMP_MIPFILTER);
        STATEMANAGER.RestoreSamplerState(i, D3DSAMP_ADDRESSU);
        STATEMANAGER.RestoreSamplerState(i, D3DSAMP_ADDRESSV);
    }

    /* - YOSUN_CONTROL_CENTER [Water Wireframe] ------------ */
    if (wireframe)
    {
        STATEMANAGER.RestoreRenderState(D3DRS_FILLMODE);
    }
    /* ----------------------------------------------------- */

    /* - SHADER [WATER] ------------------------------------ */
    STATEMANAGER.SetTexture(0, nullptr);
    STATEMANAGER.SetVertexShader(nullptr);
    STATEMANAGER.SetPixelShader(nullptr);
    STATEMANAGER.SetVertexDeclaration(nullptr);
    /* ----------------------------------------------------- */

    STATEMANAGER.RestoreRenderState(D3DRS_DIFFUSEMATERIALSOURCE);
    STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
    STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CMapOutdoor::DrawWater(long patchnum)
{
    assert(NULL != m_pTerrainPatchProxyList);

    if (!m_pTerrainPatchProxyList)
    {
        return;
    }

    CTerrainPatchProxy& rkTerrainPatchProxy = m_pTerrainPatchProxyList[patchnum];

    if (!rkTerrainPatchProxy.isUsed())
    {
        return;
    }

    if (!rkTerrainPatchProxy.isWaterExists())
    {
        return;
    }

    CGraphicVertexBuffer* pkVB = rkTerrainPatchProxy.GetWaterVertexBufferPointer();

    if (!pkVB)
    {
        return;
    }

    if (!pkVB->GetD3DVertexBuffer())
    {
        return;
    }

    UINT uPriCount = rkTerrainPatchProxy.GetWaterFaceCount();

    if (!uPriCount)
    {
        return;
    }

    STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), sizeof(SWaterVertex));
    STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, uPriCount);

    ms_faceCount += uPriCount;
}
