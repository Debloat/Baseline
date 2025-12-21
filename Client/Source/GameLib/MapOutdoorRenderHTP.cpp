#include "StdAfx.h"
#include "MapOutdoor.h"

#include "../EterLib/StateManager.h"

void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatch()
{
    //////////////////////////////////////////////////////////////////////////
    // Render State & TextureStageState

    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP);

    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);

    CSpeedTreeWrapper::ms_bSelfShadowOn = true;
    STATEMANAGER.SetBestFiltering(0);
    STATEMANAGER.SetBestFiltering(1);

    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

    STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matWorldForCommonUse);
    STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matWorldForCommonUse);

    // Render State & TextureStageState
    //////////////////////////////////////////////////////////////////////////

    STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);

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

    auto near_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), std::make_pair(fTerrainLODNear, 0L));
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

    // terrain rendering finished
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

    std::sort(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end());

    //////////////////////////////////////////////////////////////////////////
    // Render State & TextureStageState

    STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
    STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
    STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);

    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);

    // Render State & TextureStageState
    //////////////////////////////////////////////////////////////////////////
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

    TTerrainSplatPatch & rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

    D3DXMATRIX matTexTransform, matSplatAlphaTexTransform, matSplatColorTexTransform;
    m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
    m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);
    D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
    D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matTexTransform, &m_matSplatAlpha);
    STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matSplatAlphaTexTransform);

    D3DXMATRIX matTiling;
    D3DXMatrixScaling(&matTiling, 1.0f / 640.0f, -1.0f / 640.0f, 0.0f);
    matTiling._41 = 0.0f;
    matTiling._42 = 0.0f;

    D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &matTiling);
    STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

    CGraphicVertexBuffer* pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();

    if (!pkVB)
    {
        return;
    }

    STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);

    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

    int iPrevRenderedSplatNum = m_iRenderedSplatNum;

    bool isFirst = true;

    for (DWORD j = 1; j < pTerrain->GetNumTextures(); ++j)
    {
        TTerainSplat & rSplat = rTerrainSplatPatch.Splats[j];

        if (!rSplat.Active)
        {
            continue;
        }

        if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
        {
            continue;
        }

        const TTerrainTexture & rTexture = m_TextureSet.GetTexture(j);

        D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &rTexture.m_matTransform);
        STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

        if (isFirst)
        {
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
            STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
            STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
            STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
            isFirst = false;
        }

        else
        {
            STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
            STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
            STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
        }

        std::vector<int>::iterator aIterator = std::find(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end(), (int)j);

        if (aIterator == m_RenderedTextureNumVector.end())
        {
            m_RenderedTextureNumVector.push_back(j);
        }

        ++m_iRenderedSplatNum;

        if (m_iRenderedSplatNum >= m_iSplatLimit)
        {
            break;
        }

    }

    // 그림자
    if (m_bDrawShadow)
    {
        STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

        // Stage 0: just pass current color
        STATEMANAGER.SetTexture(0, nullptr);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

        if (m_bDrawChrShadow)
        {
            STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &m_matDynamicShadow);

            STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
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

        if (m_bDrawChrShadow)
        {
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
        }

        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
        STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP);
        STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP);


        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    }

    ++m_iRenderedPatchNum;

    int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;

    m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;

}
