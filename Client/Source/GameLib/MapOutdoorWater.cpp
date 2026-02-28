#include "StdAfx.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/ResourceManager.h"

#include "MapOutdoor.h"
#include "TerrainPatch.h"

/* - SHADER [WATER] ------------------------------------ */
#include "../EterLib/Camera.h"
#include "../EterLib/GrpDevice.h"
#include "../EterLib/SkyBox.h"
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

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 3> WaterSamplers =
    { {
        { 0, ESamplerState::LinearClamp }, // Sky reflection atlas
        { 1, ESamplerState::LinearWrap  }, // Normal map
        { 2, ESamplerState::PointClamp  }  // Height map (vertex texture)
    } };

    constexpr PipelineStateDesc WaterPipeline =
    {
        ShaderID::Water,
        EDepthState::EnabledReadOnly,
        EBlendState::AlphaBlend,
        ERasterState::CullFront,   // default (wireframe override handled at runtime)
        WaterSamplers.data(),
        WaterSamplers.size()
    };
}

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

    /* - SHADER [WATER] ------------------------------------ */
    IShaderProvider const* sp = GetShaderProvider();
    if (!sp || !sp->BindPipelineState(WaterPipeline))
    {
        TraceError("Water pipeline bind failed");
        return;
    }
    /* ----------------------------------------------------- */

    CGraphicImageInstance const* skyInst = m_SkyBox.GetSkyTextureInstance();
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

    /* - YOSUN_CONTROL_CENTER [Water Wireframe] ------------ */
    const bool wireframe = GetYosunControlSettings().worldEditor.terrain.drawWaterWireFrame;

    if (wireframe)
        sp->BindRasterState(ERasterState::Wireframe);
    /* ----------------------------------------------------- */

    // RenderState
    //////////////////////////////////////////////////////////////////////////
    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;
    m_matWorldForCommonUse._43 = 0.0f;

    /* - SHADER [WATER] ------------------------------------ */
    const auto& ws = GetWaterShaderSettings();

    const FrameShaderInputs& frame = sp->GetFrameShaderInputs();
    // --- WVP ---
    D3DXMATRIX matWVP;
    sp->ComputeWorldViewProj(m_matWorldForCommonUse, matWVP);
    // --- View ---
    D3DXMATRIX matView;
    std::memcpy(&matView, frame.view.data(), sizeof(D3DXMATRIX));

    WaterShaderInputs in{};
    std::memcpy(in.vs.matrices.worldViewProj.data(), &matWVP, sizeof(D3DXMATRIX));
    std::memcpy(in.vs.matrices.view.data(), &matView, sizeof(D3DXMATRIX));
    std::memcpy(in.vs.matrices.texTransform.data(), &matTexTransformWater, sizeof(D3DXMATRIX));

    in.vs.perFrame.slot1[0] = frame.cameraPos[0];
    in.vs.perFrame.slot1[1] = frame.cameraPos[1];
    in.vs.perFrame.slot1[2] = frame.cameraPos[2];
    in.vs.perFrame.slot1[3] = 0.0f;

    in.ps.material.slot1[0] = frame.sunDir[0];
    in.ps.material.slot1[1] = frame.sunDir[1];
    in.ps.material.slot1[2] = frame.sunDir[2];
    in.ps.material.slot1[3] = 0.0f;

    in.ps.material.slot2[0] = frame.sunColor[0];
    in.ps.material.slot2[1] = frame.sunColor[1];
    in.ps.material.slot2[2] = frame.sunColor[2];
    in.ps.material.slot2[3] = 1.0f;

    in.vs.perFrame.slot0[0] = frame.windDirection[0];
    in.vs.perFrame.slot0[1] = frame.windDirection[1];
    in.vs.perFrame.slot0[2] = frame.windStrength;
    in.vs.perFrame.slot0[3] = frame.timeSeconds;

    in.settings = &ws;
    CGraphicDevice::UploadWaterConstants(in);
    /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

    /* ----------------------------------------------------- */

    for (const auto& it : m_PatchVector)
    {
        DrawWater(it.second);
    }
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
