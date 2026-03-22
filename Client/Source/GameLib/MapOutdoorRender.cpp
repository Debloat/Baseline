#include "StdAfx.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "AreaTerrain.h"
#include "TerrainQuadtree.h"

#include "../EterLib/Camera.h"
#include "../EterLib/StateManager.h"

/* - YOSUN_CONTROL_CENTER [Terrain Wireframe] ---------- */
#include "../SphereLib/YosunControlCenter.h"
/* ----------------------------------------------------- */

#include "../EterLib/GrpDevice.h"

#define MAX_RENDER_SPALT 150

CArea::TCRCWithNumberVector m_dwRenderedCRCWithNumberVector;

void CMapOutdoor::RenderTerrain()
{
    if (!IsVisiblePart(PART_TERRAIN))
    {
        return;
    }

    if (!m_bSettingTerrainVisible)
    {
        return;
    }

    // Inserted by levites
    if (!m_pTerrainPatchProxyList)
    {
        return;
    }

    CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();

    if (!pCamera)
    {
        return;
    }

    auto vv = ms_matView * ms_matProj;
    BuildViewFrustum(vv);

    D3DXVECTOR3 v3Eye = pCamera->GetEye();
    m_fXforDistanceCaculation = -v3Eye.x;
    m_fYforDistanceCaculation = -v3Eye.y;

    //////////////////////////////////////////////////////////////////////////
    // Push
    m_PatchVector.clear();

    __RenderTerrain_RecurseRenderQuadTree(m_pRootNode);

    // 거리순 정렬
    std::ranges::sort(m_PatchVector);

    __RenderTerrain_RenderHardwareTransformPatch();
}

void CMapOutdoor::__RenderTerrain_RecurseRenderQuadTree(CTerrainQuadtreeNode *Node, bool bCullCheckNeed)
{
    if (bCullCheckNeed)
    {
        switch (__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(Node->center, Node->radius))
        {
            case VIEW_ALL:
                // all child nodes need not cull check
                bCullCheckNeed = false;
                break;

            case VIEW_PART:
                break;

            case VIEW_NONE:
                // no need to render
                return;
        }

        // if no need cull check more
        // -> bCullCheckNeed = false;
    }

    if (Node->Size == 1)
    {
        D3DXVECTOR3 v3Center = Node->center;
        float fDistance = fMAX(fabs(v3Center.x + m_fXforDistanceCaculation), fabs(-v3Center.y + m_fYforDistanceCaculation));
        __RenderTerrain_AppendPatch(v3Center, fDistance, Node->PatchNum);
    }

    else
    {
        if (Node->NW_Node != NULL)
        {
            __RenderTerrain_RecurseRenderQuadTree(Node->NW_Node, bCullCheckNeed);
        }

        if (Node->NE_Node != NULL)
        {
            __RenderTerrain_RecurseRenderQuadTree(Node->NE_Node, bCullCheckNeed);
        }

        if (Node->SW_Node != NULL)
        {
            __RenderTerrain_RecurseRenderQuadTree(Node->SW_Node, bCullCheckNeed);
        }

        if (Node->SE_Node != NULL)
        {
            __RenderTerrain_RecurseRenderQuadTree(Node->SE_Node, bCullCheckNeed);
        }
    }
}

int	CMapOutdoor::__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(const D3DXVECTOR3 & c_v3Center, const float& c_fRadius)
{
    const int count = 6;

    D3DXVECTOR3 center = c_v3Center;
    center.y = -center.y;

    int i;

    float distance[count];

    for (i = 0; i < count; ++i)
    {
        distance[i] = D3DXPlaneDotCoord(&m_plane[i], &center);

        if (distance[i] <= -c_fRadius)
        {
            return VIEW_NONE;
        }
    }

    for (i = 0; i < count; ++i)
    {
        if (distance[i] <= c_fRadius)
        {
            return VIEW_PART;
        }
    }

    return VIEW_ALL;
}

void CMapOutdoor::__RenderTerrain_AppendPatch(const D3DXVECTOR3& c_rv3Center, float fDistance, long lPatchNum)
{
    assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::__RenderTerrain_AppendPatch");

    if (!m_pTerrainPatchProxyList[lPatchNum].isUsed())
    {
        return;
    }

    m_pTerrainPatchProxyList[lPatchNum].SetCenterPosition(c_rv3Center);
    m_PatchVector.push_back(std::make_pair(fDistance, lPatchNum));
}

// 2004. 2. 17. myevan. 모든 부분을 보이게 초기화 한다
void CMapOutdoor::InitializeVisibleParts()
{
    m_dwVisiblePartFlags = 0xffffffff;
}

// 2004. 2. 17. myevan. 특정 부분을 보이게 하거나 감추는 함수
void CMapOutdoor::SetVisiblePart(int ePart, bool isVisible)
{
    DWORD dwMask = (1 << ePart);

    if (isVisible)
    {
        m_dwVisiblePartFlags |= dwMask;
    }

    else
    {
        DWORD dwReverseMask = ~dwMask;
        m_dwVisiblePartFlags &= dwReverseMask;
    }
}

// 2004. 2. 17. myevan. 특정 부분이 보이는지 알아내는 함수
bool CMapOutdoor::IsVisiblePart(int ePart)
{
    DWORD dwMask = (1 << ePart);

    if (dwMask & m_dwVisiblePartFlags)
    {
        return true;
    }

    return false;
}

// Splat 개수 제한
void CMapOutdoor::SetSplatLimit(int iSplatNum)
{
    m_iSplatLimit = iSplatNum;
}

std::vector<int>& CMapOutdoor::GetRenderedSplatNum(int* piPatch, int* piSplat, float* pfSplatRatio)
{
    *piPatch = m_iRenderedPatchNum;
    *piSplat = m_iRenderedSplatNum;
    *pfSplatRatio = m_iRenderedSplatNumSqSum / float(m_iRenderedPatchNum);

    return m_RenderedTextureNumVector;
}

CArea::TCRCWithNumberVector& CMapOutdoor::GetRenderedGraphicThingInstanceNum(DWORD * pdwGraphicThingInstanceNum, DWORD * pdwCRCNum)
{
    *pdwGraphicThingInstanceNum = m_dwRenderedGraphicThingInstanceNum;
    *pdwCRCNum = m_dwRenderedCRCNum;

    return m_dwRenderedCRCWithNumberVector;
}

void CMapOutdoor::RenderCollision()
{
    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        CArea * pArea;

        if (GetAreaPointer(i, &pArea))
        {
            pArea->RenderCollision();
        }
    }
}

void CMapOutdoor::RenderSky()
{
    if (IsVisiblePart(PART_SKY))
    {
        m_SkyBox.Render();
    }
}

void CMapOutdoor::RenderCloud()
{
    if (IsVisiblePart(PART_CLOUD))
    {
        m_SkyBox.RenderCloud();
    }
}

void CMapOutdoor::RenderTree()
{
    if (IsVisiblePart(PART_TREE))
    {
        CSpeedTreeForestDirectX9::Instance().Render();
    }
}

void CMapOutdoor::OnRender()
{
    /* - YOSUN_CONTROL_CENTER [Terrain Wireframe] ---------- */
    SetWireframe(GetYosunControlSettings().worldEditor.terrain.drawTerrainWireFrame);
    /* ----------------------------------------------------- */

    /* - YOSUN_CONTROL_CENTER [Patch Grid] ----------------- */
    if (GetYosunControlSettings().worldEditor.terrain.drawPatchGrid)
    {
        OnRenderPatchGrid();
    }
    /* ----------------------------------------------------- */

#ifdef __PERFORMANCE_CHECKER__
    DWORD t2 = ELTimer_GetMSec();
#endif

    RenderArea();
#ifdef __PERFORMANCE_CHECKER__
    DWORD t3 = ELTimer_GetMSec();
#endif
    if (!m_bEnableTerrainOnlyForHeight)
    {
        RenderTerrain();
    }
#ifdef __PERFORMANCE_CHECKER__
    DWORD t4 = ELTimer_GetMSec();
#endif
    RenderTree();
#ifdef __PERFORMANCE_CHECKER__
    DWORD t5 = ELTimer_GetMSec();
#endif
    RenderBlendArea();

#ifdef __PERFORMANCE_CHECKER__
    DWORD tEnd = ELTimer_GetMSec();

    if (tEnd - t1 < 7)
    {
        return;
    }

    static FILE* fp = fopen("perf_map_render.txt", "w");
    fprintf(fp, "MAP.Total %d (Time %d)\n", tEnd - t1, ELTimer_GetMSec());
    fprintf(fp, "MAP.ENV %d\n", t2 - t1);
    fprintf(fp, "MAP.OBJ %d\n", t3 - t2);
    fprintf(fp, "MAP.TRN %d\n", t4 - t3);
    fprintf(fp, "MAP.TRE %d\n", t5 - t4);
#endif
}

void CMapOutdoor::RenderEffect()
{
    if (!IsVisiblePart(PART_OBJECT))
    {
        return;
    }

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        CArea * pArea;

        if (GetAreaPointer(i, &pArea))
        {
            pArea->RenderEffect();
        }
    }
}

struct CMapOutdoor_LessThingInstancePtrRenderOrder
{
    bool operator()(CGraphicThingInstance* pkLeft, CGraphicThingInstance* pkRight)
    {
        //TODO : Camera위치기반으로 소팅
        CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
        const D3DXVECTOR3 & c_rv3CameraPos = pCurrentCamera->GetEye();
        const D3DXVECTOR3 & c_v3LeftPos  = pkLeft->GetPosition();
        const D3DXVECTOR3 & c_v3RightPos = pkRight->GetPosition();
        const auto vv = D3DXVECTOR3(c_rv3CameraPos - c_v3RightPos);
        const auto vv2 = D3DXVECTOR3(c_rv3CameraPos - c_v3LeftPos);

        return D3DXVec3LengthSq(&vv2) < D3DXVec3LengthSq(&vv);
    }
};

struct CMapOutdoor_FOpaqueThingInstanceRender
{
    inline void operator()(CGraphicThingInstance * pkThingInst)
    {
        pkThingInst->Render();
    }
};

struct CMapOutdoor_FBlendThingInstanceRender
{
    inline void operator()(CGraphicThingInstance * pkThingInst)
    {
        pkThingInst->BlendRender();
    }
};

void CMapOutdoor::RenderArea()
{
    if (!IsVisiblePart(PART_OBJECT))
    {
        return;
    }

    m_dwRenderedCRCNum = 0;
    m_dwRenderedGraphicThingInstanceNum = 0;
    m_dwRenderedCRCWithNumberVector.clear();

    // NOTE - 20041201.levites.던젼 그림자 추가
    for (int j = 0; j < AROUND_AREA_NUM; ++j)
    {
        CArea * pArea;

        if (GetAreaPointer(j, &pArea))
        {
            pArea->RenderDungeon();
        }
    }

    bool m_isDisableSortRendering = false;

    if (m_isDisableSortRendering)
    {
        for (int i = 0; i < AROUND_AREA_NUM; ++i)
        {
            CArea * pArea;

            if (GetAreaPointer(i, &pArea))
            {
                pArea->Render();

                m_dwRenderedCRCNum += pArea->DEBUG_GetRenderedCRCNum();
                m_dwRenderedGraphicThingInstanceNum += pArea->DEBUG_GetRenderedGrapphicThingInstanceNum();

                CArea::TCRCWithNumberVector & rCRCWithNumberVector = pArea->DEBUG_GetRenderedCRCWithNumVector();

                auto aIterator = rCRCWithNumberVector.begin();

                while (aIterator != rCRCWithNumberVector.end())
                {
                    DWORD dwCRC = (*aIterator++).dwCRC;

                    auto aCRCWithNumberVectorIterator = std::ranges::find_if(m_dwRenderedCRCWithNumberVector, CArea::FFindIfCRC(dwCRC));

                    if (m_dwRenderedCRCWithNumberVector.end() == aCRCWithNumberVectorIterator)
                    {
                        CArea::TCRCWithNumber aCRCWithNumber;
                        aCRCWithNumber.dwCRC = dwCRC;
                        aCRCWithNumber.dwNumber = 1;
                        m_dwRenderedCRCWithNumberVector.push_back(aCRCWithNumber);
                    }

                    else
                    {
                        CArea::TCRCWithNumber & rCRCWithNumber = *aCRCWithNumberVectorIterator;
                        rCRCWithNumber.dwNumber += 1;
                    }
                }
            }
        }

        std::ranges::sort(m_dwRenderedCRCWithNumberVector, CArea::CRCNumComp());
    }

    else
    {
        static std::vector<CGraphicThingInstance*> s_kVct_pkOpaqueThingInstSort;
        s_kVct_pkOpaqueThingInstSort.clear();

        for (int i = 0; i < AROUND_AREA_NUM; ++i)
        {
            CArea * pArea;

            if (GetAreaPointer(i, &pArea))
            {
                pArea->CollectRenderingObject(s_kVct_pkOpaqueThingInstSort);
            }

        }

        std::ranges::sort(s_kVct_pkOpaqueThingInstSort, CMapOutdoor_LessThingInstancePtrRenderOrder());
        std::ranges::for_each(s_kVct_pkOpaqueThingInstSort, CMapOutdoor_FOpaqueThingInstanceRender());
    }
}

void CMapOutdoor::RenderBlendArea()
{
    if (!IsVisiblePart(PART_OBJECT))
    {
        return;
    }

    static std::vector<CGraphicThingInstance*> s_kVct_pkBlendThingInstSort;
    s_kVct_pkBlendThingInstSort.clear();

    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        CArea * pArea;

        if (GetAreaPointer(i, &pArea))
        {
            pArea->CollectBlendRenderingObject(s_kVct_pkBlendThingInstSort);
        }
    }

    if (!s_kVct_pkBlendThingInstSort.empty())
    {
        std::ranges::sort(s_kVct_pkBlendThingInstSort, CMapOutdoor_LessThingInstancePtrRenderOrder());

        STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, TRUE);
        STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
        STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

        std::ranges::for_each(s_kVct_pkBlendThingInstSort, CMapOutdoor_FBlendThingInstanceRender());

        STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
        STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
        STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
    }
}

void CMapOutdoor::RenderDungeon()
{
    for (int i = 0; i < AROUND_AREA_NUM; ++i)
    {
        CArea * pArea;

        if (!GetAreaPointer(i, &pArea))
        {
            continue;
        }

        pArea->RenderDungeon();
    }
}

void CMapOutdoor::SelectIndexBuffer(BYTE byLODLevel, WORD * pwPrimitiveCount, D3DPRIMITIVETYPE * pePrimitiveType)
{
    if (0 == byLODLevel)
    {
        *pwPrimitiveCount = m_wNumIndices[byLODLevel] - 2;
        *pePrimitiveType = D3DPT_TRIANGLESTRIP;
    }

    else
    {
        *pwPrimitiveCount =  m_wNumIndices[byLODLevel] / 3;
        *pePrimitiveType = D3DPT_TRIANGLELIST;
    }

    STATEMANAGER.SetIndices(m_IndexBuffer[byLODLevel].GetD3DIndexBuffer(), 0);
}

struct FPatchNumMatch
{
    long m_lPatchNumToCheck;
    FPatchNumMatch(long lPatchNum)
    {
        m_lPatchNumToCheck = lPatchNum;
    }

    bool operator()(std::pair<long, BYTE> aPair)
    {
        return m_lPatchNumToCheck == aPair.first;
    }
};

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> ScreenPrimitiveSamplers =
    { {
        { 0, ESamplerState::LinearClamp }
    } };

    constexpr PipelineStateDesc TerrainWireframePipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::EnabledReadOnly,
        EBlendState::Opaque,
        ERasterState::Wireframe,
        ScreenPrimitiveSamplers.data(),
        ScreenPrimitiveSamplers.size()
    };
}

void CMapOutdoor::DrawWireFrame(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
    assert(NULL != m_pTerrainPatchProxyList && "CMapOutdoor::DrawWireFrame");

    CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

    if (!pTerrainPatchProxy->isUsed())
        return;

    long sPatchNum = pTerrainPatchProxy->GetPatchNum();
    if (sPatchNum < 0)
        return;

    BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
    if (0xFF == ucTerrainNum)
        return;

    const IShaderProvider* sp = GetShaderProvider();

    if (!sp || !sp->BindPipelineState(TerrainWireframePipeline))
        return;

    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);

    ScreenPrimitiveShaderInputs in{};

    // world = identity
    std::memcpy(in.vs.world.data(), &matIdentity, sizeof(D3DXMATRIX));

    // viewProj
    D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();
    std::memcpy(in.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));

    in.ps.mode[0] = 0.0f;
    in.ps.mode[1] = 1.0f;

    CGraphicDevice::UploadScreenPrimitiveConstants(in);

    STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
}

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> TerrainMarkedAreaSamplers =
    { {
        { 0, ESamplerState::LinearWrap }
    } };

    constexpr PipelineStateDesc TerrainMarkedAreaPipeline =
    {
        ShaderID::TerrainMarkedArea,
        EDepthState::EnabledReadOnly,
        EBlendState::AlphaBlend,
        ERasterState::CullFront,
        TerrainMarkedAreaSamplers.data(),
        TerrainMarkedAreaSamplers.size()
    };
}

// Attr
void CMapOutdoor::RenderMarkedArea()
{
    if (!m_pTerrainPatchProxyList)
        return;

    const IShaderProvider* sp = GetShaderProvider();

    if (!sp || !sp->BindPipelineState(TerrainMarkedAreaPipeline))
        return;

    WORD wPrimitiveCount;
    D3DPRIMITIVETYPE eType;
    SelectIndexBuffer(0, &wPrimitiveCount, &eType);

    static long lStartTime = timeGetTime();

    float fTime = float((timeGetTime() - lStartTime) % 3000) / 3000.0f;
    float fAlpha = fabs(fTime - 0.5f) / 2.0f + 0.1f;

    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);

    TerrainMarkedAreaShaderInputs in{};

    // world = identity
    std::memcpy(in.vs.world.data(), &matIdentity, sizeof(D3DXMATRIX));

    // viewProj
    D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();
    std::memcpy(in.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));
    std::memcpy(in.vs.viewInverse.data(), &m_matViewInverse, sizeof(D3DXMATRIX));

    in.vs.texScale[0] = m_fTerrainTexCoordBase * 32.0f;

    in.ps.alpha[0] = fAlpha;

    CGraphicDevice::UploadTerrainMarkedAreaConstants(in);

    STATEMANAGER.SetTexture(0, m_attrImageInstance.GetTexturePointer()->GetD3DTexture());

    RecurseRenderAttr(m_pRootNode);
}

void CMapOutdoor::RecurseRenderAttr(CTerrainQuadtreeNode *Node, bool bCullEnable)
{
    if (bCullEnable)
    {
        if (__RenderTerrain_RecurseRenderQuadTree_CheckBoundingCircle(Node->center, Node->radius) == VIEW_NONE)
        {
            return;
        }
    }

    {
        if (Node->Size == 1)
        {
            DrawPatchAttr(Node->PatchNum);
        }

        else
        {
            if (Node->NW_Node != NULL)
            {
                RecurseRenderAttr(Node->NW_Node, bCullEnable);
            }

            if (Node->NE_Node != NULL)
            {
                RecurseRenderAttr(Node->NE_Node, bCullEnable);
            }

            if (Node->SW_Node != NULL)
            {
                RecurseRenderAttr(Node->SW_Node, bCullEnable);
            }

            if (Node->SE_Node != NULL)
            {
                RecurseRenderAttr(Node->SE_Node, bCullEnable);
            }
        }
    }
}

void CMapOutdoor::DrawPatchAttr(long patchnum)
{
    CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

    if (!pTerrainPatchProxy->isUsed())
        return;

    long sPatchNum = pTerrainPatchProxy->GetPatchNum();
    if (sPatchNum < 0)
        return;

    BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
    if (ucTerrainNum == 0xFF)
        return;

    CTerrain* pTerrain;
    if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
        return;

    if (!pTerrain->IsMarked())
        return;

    STATEMANAGER.SetStreamSource(
        0,
        pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr()->GetD3DVertexBuffer(),
        m_iPatchTerrainVertexSize
    );

    STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_iPatchTerrainVertexCount, 0, m_wNumIndices[0] - 2);
}
