#include "StdAfx.h"
#include "MapOutdoor.h"

#include "../EterLib/StateManager.h"

#include "../EterLib/GrpDevice.h"

#include "../SphereLib/YosunControlCenter.h"

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 2> TerrainSamplers =
    { {
        { 0, ESamplerState::LinearWrap },   // terrain texture
        { 1, ESamplerState::LinearClamp }   // splat alpha
    } };

    constexpr PipelineStateDesc TerrainPipeline =
    {
        ShaderID::Terrain,
        EDepthState::EnabledWrite,
        EBlendState::AlphaBlend,
        ERasterState::CullFront,
        TerrainSamplers.data(),
        TerrainSamplers.size()
    };
}

void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatch()
{
    IShaderProvider const* sp = GetShaderProvider();

    if (!sp || !sp->BindPipelineState(TerrainPipeline))
    {
        TraceError("Terrain pipeline bind failed");
        return;
    }

    CSpeedTreeWrapper::ms_bSelfShadowOn = true;

    auto& terrainMetrics = GetYosunControlSettings().terrainMetrics;
    terrainMetrics.totalDrawCalls = 0;
    terrainMetrics.renderedPatches = 0;
    terrainMetrics.renderedSplats = 0;

    m_iRenderedSplatNumSqSum = 0;
    m_iRenderedPatchNum = 0;
    m_iRenderedSplatNum = 0;
    m_RenderedTextureNumVector.clear();

    // [KaptanYosun Dev Note] Terrain LOD distances derived from current visible patch range (tracks view distance / fog automatically)
    float fTerrainLODNear = 0.0f;
    float fTerrainLODMid = 0.0f;

    if (!m_PatchVector.empty())
    {
        const float fMaxVisiblePatchDistance = m_PatchVector.back().first;

        // Geometry LOD thresholds (material is identical for all LODs)
        // LOD0: high detail geometry (near)    00%  .. 40% of visible patch range
        // LOD1: medium detail geometry         40%  .. 75%
        // LOD2: low detail geometry (far)      75%  .. 100%
        fTerrainLODNear = max(1.0f, fMaxVisiblePatchDistance * 0.40f);
        fTerrainLODMid = max(fTerrainLODNear + 1.0f, fMaxVisiblePatchDistance * 0.75f);
    }

    auto near_it = std::ranges::upper_bound(m_PatchVector, std::make_pair(fTerrainLODNear, 0L));
    auto far_it = m_PatchVector.end();

    WORD wPrimitiveCount;
    D3DPRIMITIVETYPE ePrimitiveType;

    BYTE byCUrrentLODLevel = 0;

    SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

    auto it = m_PatchVector.begin();

    // NOTE: 맵툴에서는 view ~ fog near 사이의 지형을 fog disabled 상태로 그리는 작업을 하지 않음.
    for (; it != near_it; ++it)
    {
        if (byCUrrentLODLevel == 0 && fTerrainLODNear <= it->first)
        {
            byCUrrentLODLevel = 1;
            SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
        }

        else if (byCUrrentLODLevel == 1 && fTerrainLODMid <= it->first)
        {
            byCUrrentLODLevel = 2;
            SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
        }

        __HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount, ePrimitiveType);

        if (m_iRenderedSplatNum >= m_iSplatLimit)
        {
            break;
        }

        if (m_bDrawWireFrame)
        {
            DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
        }
    }

    if (m_iRenderedSplatNum < m_iSplatLimit)
    {
        for (it = near_it; it != far_it; ++it)
        {
            if (byCUrrentLODLevel == 0 && fTerrainLODNear <= it->first)
            {
                byCUrrentLODLevel = 1;
                SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
            }

            else if (byCUrrentLODLevel == 1 && fTerrainLODMid <= it->first)
            {
                byCUrrentLODLevel = 2;
                SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
            }

            __HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount, ePrimitiveType);

            if (m_iRenderedSplatNum >= m_iSplatLimit)
            {
                break;
            }

            if (m_bDrawWireFrame)
            {
                DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
            }
        }
    }

    std::ranges::sort(m_RenderedTextureNumVector);
}

void CMapOutdoor::__HardwareTransformPatch_RenderPatchSplat(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
    assert(NULL != m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchSplat");
    CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

    if (!pTerrainPatchProxy->isUsed())
    {
        return;
    }

    long sPatchNum = pTerrainPatchProxy->GetPatchNum();

    if (sPatchNum < 0)
    {
        return;
    }

    BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();

    if (0xFF == ucTerrainNum)
    {
        return;
    }

    CTerrain * pTerrain;

    if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
    {
        return;
    }

    WORD wCoordX, wCoordY;
    pTerrain->GetCoordinate(&wCoordX, &wCoordY);

    TTerrainSplatPatch& rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

    IShaderProvider const* sp = GetShaderProvider();
    if (!sp)
    {
        return;
    }

    D3DXMATRIX matPatchTranslate;
    D3DXMATRIX matSplatAlphaTexTransform;
    D3DXMATRIX matSplatColorTexTransform;

    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);

    D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();

    D3DXMatrixIdentity(&matPatchTranslate);
    matPatchTranslate._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
    matPatchTranslate._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);

    D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matPatchTranslate, &m_matSplatAlpha);

    CGraphicVertexBuffer* pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();

    if (!pkVB)
    {
        return;
    }

    STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);

    int iPrevRenderedSplatNum = m_iRenderedSplatNum;

    bool isFirst = true;

    std::array<DWORD, 32> activeSplats;
    DWORD activeSplatCount = 0;

    for (DWORD j = 1; j < pTerrain->GetNumTextures(); ++j)
    {
        TTerainSplat& rSplat = rTerrainSplatPatch.Splats[j];

        if (!rSplat.Active)
        {
            continue;
        }

        if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
        {
            continue;
        }

        activeSplats[activeSplatCount++] = j;
    }

    for (DWORD k = 0; k < activeSplatCount; ++k)
    {
        DWORD j = activeSplats[k];

        TTerainSplat& rSplat = rTerrainSplatPatch.Splats[j];

        const TTerrainTexture& rTexture = m_TextureSet.GetTexture(j);

        std::memcpy(&matSplatColorTexTransform, &rTexture.m_matTransform, sizeof(D3DXMATRIX));

        TerrainShaderInputs in{};
        std::memcpy(in.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));
        std::memcpy(in.vs.world.data(), &matIdentity, sizeof(D3DXMATRIX));
        std::memcpy(in.vs.colorTexMatrix.data(), &matSplatColorTexTransform, sizeof(D3DXMATRIX));
        std::memcpy(in.vs.alphaTexMatrix.data(), &matSplatAlphaTexTransform, sizeof(D3DXMATRIX));

        in.ps.layerState[0] = isFirst ? 0.0f : 1.0f;
        in.ps.layerState[1] = 0.0f;
        in.ps.layerState[2] = 0.0f;
        in.ps.layerState[3] = 0.0f;

        CGraphicDevice::UploadTerrainConstants(in);

        STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
        STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
        STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

        auto& terrainMetrics = GetYosunControlSettings().terrainMetrics;
        ++terrainMetrics.totalDrawCalls;

        if (terrainMetrics.totalDrawCalls > terrainMetrics.peakDrawCalls)
        {
            terrainMetrics.peakDrawCalls = terrainMetrics.totalDrawCalls;
        }

        if (isFirst)
        {
            isFirst = false;
        }

        if (auto aIterator = std::ranges::find(m_RenderedTextureNumVector, (int)j); aIterator == m_RenderedTextureNumVector.end())
        {
            m_RenderedTextureNumVector.push_back(j);
        }

        ++m_iRenderedSplatNum;
        GetYosunControlSettings().terrainMetrics.renderedSplats = m_iRenderedSplatNum;

        if (m_iRenderedSplatNum >= m_iSplatLimit)
        {
            break;
        }
    }

    /* - SHADOWS ------------------------------------------- */
    if (m_bDrawShadow)
    {
        STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

        // Stage 0: just pass current color
        STATEMANAGER.SetTexture(0, NULL);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

        if (m_bDrawChrShadow)
        {
            STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &m_matDynamicShadow);

            STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }
        else
        {
            STATEMANAGER.SetTexture(1, NULL);
        }

        ms_faceCount += wPrimitiveCount;
        STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
        ++m_iRenderedSplatNum;
        GetYosunControlSettings().terrainMetrics.renderedSplats = m_iRenderedSplatNum;

        if (m_bDrawChrShadow)
        {
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        }

        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    }
    /* ----------------------------------------------------- */

    ++m_iRenderedPatchNum;
    GetYosunControlSettings().terrainMetrics.renderedPatches = m_iRenderedPatchNum;

    int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;

    m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;

}
