#include "StdAfx.h"
#include "MapOutdoor.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/GrpBase.h"
#include "../TerrainLib/Terrain.h"

/* - YOSUN_CONTROL_CENTER [Patch Grid] ----------------- */
void CMapOutdoor::OnRenderPatchGrid()
{
    SetDiffuseColor(1.0f, 0.0f, 1.0f);

    const long viewRadius = GetViewRadius();

    CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
    if (!pCamera)
        return;

    const D3DXVECTOR3& camPos = pCamera->GetTarget();

    constexpr WORD wGridWidth = CTerrainImpl::PATCH_XSIZE;
    const float fGridWidth = float(wGridWidth * CTerrainImpl::CELLSCALE);

    const long minX = ((long(camPos.x) / CTerrainImpl::CELLSCALE - viewRadius) / wGridWidth) * wGridWidth;
    const long maxX = ((long(camPos.x) / CTerrainImpl::CELLSCALE + viewRadius) / wGridWidth) * wGridWidth;
    const long minY = ((-long(camPos.y) / CTerrainImpl::CELLSCALE - viewRadius) / wGridWidth) * wGridWidth;
    const long maxY = ((-long(camPos.y) / CTerrainImpl::CELLSCALE + viewRadius) / wGridWidth) * wGridWidth;

    for (long x = minX; x <= maxX; x += wGridWidth)
    {
        const float fx = float(x * CTerrainImpl::CELLSCALE);
        for (long y = minY; y <= maxY; y += wGridWidth)
        {
            const float fy = float(y * CTerrainImpl::CELLSCALE);
            const float z0 = GetHeight(fx, fy) + 10.0f;
            const float z1 = GetHeight(fx, fy + fGridWidth) + 10.0f;

            RenderLine3d(fx, -fy, z0, fx, -fy - fGridWidth, z1);
        }
    }

    for (long y = minY; y <= maxY; y += wGridWidth)
    {
        const float fy = float(y * CTerrainImpl::CELLSCALE);
        for (long x = minX; x <= maxX; x += wGridWidth)
        {
            const float fx = float(x * CTerrainImpl::CELLSCALE);
            const float z0 = GetHeight(fx, fy) + 10.0f;
            const float z1 = GetHeight(fx + fGridWidth, fy) + 10.0f;

            RenderLine3d(fx, -fy, z0, fx + fGridWidth, -fy, z1);
        }
    }
}
/* ----------------------------------------------------- */
