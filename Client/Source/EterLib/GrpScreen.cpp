#include "StdAfx.h"
#include "GrpScreen.h"

#include <bit>

#include "Camera.h"
#include "StateManager.h"

/* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
#include "ImGui/imgui_impl_dx9.h"
#endif
/* ----------------------------------------------------- */

#include "ShaderProvider.h"
#include "ShaderVertexDeclarations.h"
#include "ShaderParameters.h"
#include "GrpDevice.h"

DWORD		CScreen::ms_diffuseColor = 0xffffffff;
DWORD		CScreen::ms_clearColor = 0L;
DWORD		CScreen::ms_clearStencil = 0L;
float		CScreen::ms_clearDepth = 1.0f;
Frustum		CScreen::ms_frustum;

namespace
{
    constexpr std::array<PipelineStateDesc::SamplerBinding, 1> kScreenPrimitiveSamplers =
    { {
        { 0, ESamplerState::LinearClamp }
    } };

    // 3D solid debug
    constexpr PipelineStateDesc kScreenPrimitive3DSolidPipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::EnabledReadOnly,
        EBlendState::Opaque,
        ERasterState::CullNone,
        kScreenPrimitiveSamplers.data(),
        kScreenPrimitiveSamplers.size()
    };

    // 3D wire debug
    constexpr PipelineStateDesc kScreenPrimitive3DWirePipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::EnabledReadOnly,
        EBlendState::Opaque,
        ERasterState::Wireframe,
        kScreenPrimitiveSamplers.data(),
        kScreenPrimitiveSamplers.size()
    };

    // 2D overlay
    constexpr PipelineStateDesc kScreenPrimitive2DPipeline =
    {
        ShaderID::ScreenPrimitive,
        EDepthState::Disabled,
        EBlendState::AlphaBlend,
        ERasterState::CullNone,
        kScreenPrimitiveSamplers.data(),
        kScreenPrimitiveSamplers.size()
    };
}

void CScreen::RenderLine3d(float sx, float sy, float sz, float ex, float ey, float ez)
{
    assert(ms_lpd3dDevice != NULL);

    std::array<SPDTVertexRaw, 2> vertices{ {
        { sx, sy, sz, ms_diffuseColor, 0.0f, 0.0f },
        { ex, ey, ez, ms_diffuseColor, 0.0f, 0.0f }
    } };

    if (SetPDTStream(vertices.data(), 2))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive3DSolidPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive3D(*ms_lpd3dMatStack->GetTop(), in);
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        STATEMANAGER.DrawPrimitive(D3DPT_LINELIST, 0, 1);
    }
}

void CScreen::RenderBar3d(const D3DXVECTOR3* c_pv3Positions)
{
    assert(ms_lpd3dDevice != NULL);

    std::array<SPDTVertexRaw, 4> vertices{ {
        { c_pv3Positions[0].x, c_pv3Positions[0].y, c_pv3Positions[0].z, ms_diffuseColor, 0.0f, 0.0f },
        { c_pv3Positions[2].x, c_pv3Positions[2].y, c_pv3Positions[2].z, ms_diffuseColor, 0.0f, 0.0f },
        { c_pv3Positions[1].x, c_pv3Positions[1].y, c_pv3Positions[1].z, ms_diffuseColor, 0.0f, 0.0f },
        { c_pv3Positions[3].x, c_pv3Positions[3].y, c_pv3Positions[3].z, ms_diffuseColor, 0.0f, 0.0f },
    } };

    if (SetPDTStream(vertices.data(), 4))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive3DSolidPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive3D(*ms_lpd3dMatStack->GetTop(), in);
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
}

void CScreen::RenderCube(float sx, float sy, float sz, float ex, float ey, float ez)
{
    std::array<SPDTVertexRaw, 8> vertices{ {
        { sx, sy, sz, ms_diffuseColor, 0.0f, 0.0f },
        { ex, sy, sz, ms_diffuseColor, 0.0f, 0.0f },
        { sx, ey, sz, ms_diffuseColor, 0.0f, 0.0f },
        { ex, ey, sz, ms_diffuseColor, 0.0f, 0.0f },
        { sx, sy, ez, ms_diffuseColor, 0.0f, 0.0f },
        { ex, sy, ez, ms_diffuseColor, 0.0f, 0.0f },
        { sx, ey, ez, ms_diffuseColor, 0.0f, 0.0f },
        { ex, ey, ez, ms_diffuseColor, 0.0f, 0.0f },
    } };

    if (SetPDTStream(vertices.data(), 8))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive3DSolidPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive3D(*ms_lpd3dMatStack->GetTop(), in);
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        SetDefaultIndexBuffer(DEFAULT_IB_FILL_CUBE);
        STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 8, 0, 4 * 3);
    }
}

void CScreen::RenderCube(float sx, float sy, float sz, float ex, float ey, float ez, D3DXMATRIX matRotation)
{
    D3DXVECTOR3 v3Center((sx + ex) * 0.5f, (sy + ey) * 0.5f, (sz + ez) * 0.5f);
    std::array<D3DXVECTOR3, 8> v3Vertex{ {
        { sx, sy, sz }, { ex, sy, sz }, { sx, ey, sz }, { ex, ey, sz },
        { sx, sy, ez }, { ex, sy, ez }, { sx, ey, ez }, { ex, ey, ez }
    } };

    std::array<SPDTVertexRaw, 8> vertices{};

    for (std::size_t i = 0; i < v3Vertex.size(); ++i)
    {
        v3Vertex[i] -= v3Center;
        D3DXVec3TransformCoord(&v3Vertex[i], &v3Vertex[i], &matRotation);
        v3Vertex[i] += v3Center;

        vertices[i].px = v3Vertex[i].x;
        vertices[i].py = v3Vertex[i].y;
        vertices[i].pz = v3Vertex[i].z;
        vertices[i].diffuse = ms_diffuseColor;
        vertices[i].u = 0.0f;
        vertices[i].v = 0.0f;
    }

    if (SetPDTStream(vertices.data(), 8))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive3DSolidPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive3D(*ms_lpd3dMatStack->GetTop(), in);
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        SetDefaultIndexBuffer(DEFAULT_IB_FILL_CUBE);
        STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 8, 0, 4 * 3);
    }
}

void CScreen::RenderLine2d(float sx, float sy, float ex, float ey, float z)
{
    assert(ms_lpd3dDevice != NULL);

    std::array<SPDTVertexRaw, 2> vertices{ {
        { sx, sy, z, ms_diffuseColor, 0.0f, 0.0f },
        { ex, ey, z, ms_diffuseColor, 0.0f, 0.0f }
    } };

    if (SetPDTStream(vertices.data(), 2))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive2DPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive2DOrthoPixel(
            static_cast<float>(ms_Viewport.Width),
            static_cast<float>(ms_Viewport.Height),
            in
        );
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        STATEMANAGER.DrawPrimitive(D3DPT_LINELIST, 0, 1);
    }
}

void CScreen::RenderBox2d(float sx, float sy, float ex, float ey, float z)
{
    assert(ms_lpd3dDevice != NULL);

    std::array<SPDTVertexRaw, 8> vertices{ {
        { sx, sy, z, ms_diffuseColor, 0.0f, 0.0f },
        { ex, sy, z, ms_diffuseColor, 0.0f, 0.0f },

        { sx, sy, z, ms_diffuseColor, 0.0f, 0.0f },
        { sx, ey, z, ms_diffuseColor, 0.0f, 0.0f },

        { ex, sy, z, ms_diffuseColor, 0.0f, 0.0f },
        { ex, ey, z, ms_diffuseColor, 0.0f, 0.0f },

        { sx, ey, z, ms_diffuseColor, 0.0f, 0.0f },
        { ex + 1.0f, ey, z, ms_diffuseColor, 0.0f, 0.0f }
    } };

    if (SetPDTStream(vertices.data(), 8))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive2DPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive2DOrthoPixel(
            static_cast<float>(ms_Viewport.Width),
            static_cast<float>(ms_Viewport.Height),
            in
        );
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        STATEMANAGER.DrawPrimitive(D3DPT_LINELIST, 0, 4);
    }
}

void CScreen::RenderBar2d(float sx, float sy, float ex, float ey, float z)
{
    assert(ms_lpd3dDevice != NULL);

    std::array<SPDTVertexRaw, 4> vertices{ {
        { sx, sy, z, ms_diffuseColor, 0.0f, 0.0f },
        { sx, ey, z, ms_diffuseColor, 0.0f, 0.0f },
        { ex, sy, z, ms_diffuseColor, 0.0f, 0.0f },
        { ex, ey, z, ms_diffuseColor, 0.0f, 0.0f },
    } };

    if (SetPDTStream(vertices.data(), 4))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive2DPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive2DOrthoPixel(
            static_cast<float>(ms_Viewport.Width),
            static_cast<float>(ms_Viewport.Height),
            in
        );
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
}

void CScreen::RenderGradationBar2d(float sx, float sy, float ex, float ey, DWORD dwStartColor, DWORD dwEndColor, float ez)
{
    assert(ms_lpd3dDevice != NULL);

    if (sx == ex || sy == ey)
        return;

    std::array<SPDTVertexRaw, 4> vertices{ {
        { sx, sy, ez, dwStartColor, 0.0f, 0.0f },
        { sx, ey, ez, dwEndColor,   0.0f, 0.0f },
        { ex, sy, ez, dwStartColor, 0.0f, 0.0f },
        { ex, ey, ez, dwEndColor,   0.0f, 0.0f },
    } };

    if (SetPDTStream(vertices.data(), 4))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive2DPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive2DOrthoPixel(
            static_cast<float>(ms_Viewport.Width),
            static_cast<float>(ms_Viewport.Height),
            in
        );
        in.ps.mode[0] = 0.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
    }
}

void CScreen::RenderCircle2d(float fx, float fy, float fz, float fRadius, int iStep)
{
    int count;
    float theta, delta;
    float x, y, z;
    std::vector<D3DXVECTOR3> pts;

    pts.clear();
    pts.resize(iStep);

    theta = 0.0;
    delta = 2 * D3DX_PI / float(iStep);

    for (count = 0; count < iStep; count++)
    {
        x = fx + fRadius * cosf(theta);
        y = fy + fRadius * sinf(theta);
        z = fz;

        pts[count] = D3DXVECTOR3(x, y, z);

        theta += delta;
    }

    for (count = 0; count < iStep - 1; count++)
    {
        RenderLine2d(pts[count].x, pts[count].y, pts[count + 1].x, pts[count + 1].y, fz);
    }

    RenderLine2d(pts[iStep - 1].x, pts[iStep - 1].y, pts[0].x, pts[0].y, fz);
}

void CScreen::RenderCircle3d(float fx, float fy, float fz, float fRadius, int iStep)
{
    int count;
    float theta, delta;
    std::vector<D3DXVECTOR3> pts;

    pts.clear();
    pts.resize(iStep);

    theta = 0.0;
    delta = 2 * D3DX_PI / float(iStep);

    const D3DXMATRIX & c_rmatInvView = CCameraManager::Instance().GetCurrentCamera()->GetBillboardMatrix();

    for (count = 0; count < iStep; count++)
    {
        pts[count] = D3DXVECTOR3(fRadius * cosf(theta), fRadius * sinf(theta), 0.0f);
        D3DXVec3TransformCoord(&pts[count], &pts[count], &c_rmatInvView);

        theta += delta;
    }

    for (count = 0; count < iStep - 1; count++)
    {
        RenderLine3d(fx + pts[count].x, fy + pts[count].y, fz + pts[count].z,
                     fx + pts[count + 1].x, fy + pts[count + 1].y, fz + pts[count + 1].z);
    }

    RenderLine3d(fx + pts[iStep - 1].x, fy + pts[iStep - 1].y, fz + pts[iStep - 1].z,
                 fx + pts[0].x, fy + pts[0].y, fz + pts[0].z);
}

void CScreen::RenderD3DXMesh(LPD3DXMESH lpMesh, const D3DXMATRIX* c_pmatWorld, float fx, float fy, float fz, float fRadius, D3DFILLMODE d3dFillMode)
{
    assert(ms_lpd3dDevice != NULL);
    if (!lpMesh)
        return;

    D3DXMATRIX matTranslation;
    D3DXMATRIX matScaling;

    D3DXMatrixTranslation(&matTranslation, fx, fy, fz);
    D3DXMatrixScaling(&matScaling, fRadius, fRadius, fRadius);

    D3DXMATRIX matWorld = matScaling * matTranslation;

    if (c_pmatWorld)
        matWorld *= *c_pmatWorld;

    LPDIRECT3DINDEXBUFFER9 lpIndexBuffer = nullptr;
    LPDIRECT3DVERTEXBUFFER9 lpVertexBuffer = nullptr;

    lpMesh->GetIndexBuffer(&lpIndexBuffer);
    lpMesh->GetVertexBuffer(&lpVertexBuffer);

    if (!lpIndexBuffer || !lpVertexBuffer)
    {
        if (lpIndexBuffer) lpIndexBuffer->Release();
        if (lpVertexBuffer) lpVertexBuffer->Release();
        return;
    }

    const IShaderProvider* sp = GetShaderProvider();
    if (!sp)
    {
        lpIndexBuffer->Release();
        lpVertexBuffer->Release();
        return;
    }

    const PipelineStateDesc& pipeline =
        (d3dFillMode == D3DFILL_WIREFRAME)
        ? kScreenPrimitive3DWirePipeline
        : kScreenPrimitive3DSolidPipeline;

    if (!sp->BindPipelineState(pipeline))
    {
        lpIndexBuffer->Release();
        lpVertexBuffer->Release();
        return;
    }

    STATEMANAGER.SetVertexDeclaration(CShaderInputLayouts::Get(EShaderInputLayout::PNT));

    D3DXMATRIX matWVP = matWorld * ms_matView * ms_matProj;

    ScreenPrimitiveShaderInputs in{};
    std::memcpy(in.vs.worldViewProj.data(), &matWVP, sizeof(D3DXMATRIX));
    in.ps.mode[0] = 0.0f;
    in.ps.mode[1] = 1.0f;

    CGraphicDevice::UploadScreenPrimitiveConstants(in);

    STATEMANAGER.SetIndices(lpIndexBuffer, 0);
    STATEMANAGER.SetStreamSource(0, lpVertexBuffer, 24);

    STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, lpMesh->GetNumVertices(), 0, lpMesh->GetNumFaces());

    lpIndexBuffer->Release();
    lpVertexBuffer->Release();
}

void CScreen::RenderSphere(const D3DXMATRIX * c_pmatWorld, float fx, float fy, float fz, float fRadius, D3DFILLMODE d3dFillMode)
{
    RenderD3DXMesh(ms_lpSphereMesh, c_pmatWorld, fx, fy, fz, fRadius, d3dFillMode);
}

void CScreen::RenderCylinder(const D3DXMATRIX * c_pmatWorld, float fx, float fy, float fz, float fRadius, float /*fLength*/, D3DFILLMODE d3dFillMode)
{
    RenderD3DXMesh(ms_lpCylinderMesh, c_pmatWorld, fx, fy, fz, fRadius, d3dFillMode);
}

void CScreen::RenderTextureBox(float sx, float sy, float ex, float ey, float z, float su, float sv, float eu, float ev)
{
    assert(ms_lpd3dDevice != NULL);

    std::array<TPDTVertex, 4> vertices{ {
        { TPosition(sx, sy, z), ms_diffuseColor, TTextureCoordinate(su, sv) },
        { TPosition(ex, sy, z), ms_diffuseColor, TTextureCoordinate(eu, sv) },
        { TPosition(sx, ey, z), ms_diffuseColor, TTextureCoordinate(su, ev) },
        { TPosition(ex, ey, z), ms_diffuseColor, TTextureCoordinate(eu, ev) },
    } };

    SetDefaultIndexBuffer(DEFAULT_IB_FILL_RECT);

    if (SetPDTStream(vertices.data(), 4))
    {
        const IShaderProvider* sp = GetShaderProvider();
        if (!sp || !sp->BindPipelineState(kScreenPrimitive2DPipeline))
            return;

        ScreenPrimitiveShaderInputs in{};
        sp->FillScreenPrimitive2DOrthoPixel(
            static_cast<float>(ms_Viewport.Width),
            static_cast<float>(ms_Viewport.Height),
            in
        );
        in.ps.mode[0] = 1.0f;

        CGraphicDevice::UploadScreenPrimitiveConstants(in);

        STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
    }
}

void CScreen::SetCursorPosition(int x, int y, int hres, int vres)
{
    D3DXVECTOR3 v;
    v.x = -(((2.0f * x) / hres) - 1) / ms_matProj._11;
    v.y = (((2.0f * y) / vres) - 1) / ms_matProj._22;
    v.z = 1.0f;

    D3DXMATRIX matViewInverse = ms_matInverseView;
    //D3DXMatrixInverse(&matViewInverse, NULL, &ms_matView);

    ms_vtPickRayDir.x = v.x * matViewInverse._11 +
                        v.y * matViewInverse._21 +
                        v.z * matViewInverse._31;

    ms_vtPickRayDir.y = v.x * matViewInverse._12 +
                        v.y * matViewInverse._22 +
                        v.z * matViewInverse._32;

    ms_vtPickRayDir.z = v.x * matViewInverse._13 +
                        v.y * matViewInverse._23 +
                        v.z * matViewInverse._33;

    ms_vtPickRayOrig.x = matViewInverse._41;
    ms_vtPickRayOrig.y = matViewInverse._42;
    ms_vtPickRayOrig.z = matViewInverse._43;

//	// 2003. 9. 9 동현 추가
//	// 지형 picking을 위한 뻘짓... ㅡㅡ; 위에 것과 통합 필요...
    ms_Ray.SetStartPoint(ms_vtPickRayOrig);
    ms_Ray.SetDirection(-ms_vtPickRayDir, 51200.0f);
//	// 2003. 9. 9 동현 추가
}

bool CScreen::GetCursorPosition(float* px, float* py, float* pz)
{
    if (!GetCursorXYPosition(px, py))
    {
        return false;
    }

    if (!GetCursorZPosition(pz))
    {
        return false;
    }

    return true;
}

bool CScreen::GetCursorXYPosition(float* px, float* py)
{
    D3DXVECTOR3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();

    constexpr float kExtent = 90000000.0f;
    std::array<TPosition, 4> posVertices{ {
        TPosition(v3Eye.x - kExtent, v3Eye.y + kExtent, 0.0f),
        TPosition(v3Eye.x - kExtent, v3Eye.y - kExtent, 0.0f),
        TPosition(v3Eye.x + kExtent, v3Eye.y + kExtent, 0.0f),
        TPosition(v3Eye.x + kExtent, v3Eye.y - kExtent, 0.0f),
    } };

    static const WORD sc_awFillRectIndices[6] = { 0, 2, 1, 2, 3, 1, };

    float u, v, t;

    for (int i = 0; i < 2; ++i)
    {
        if (IntersectTriangle(ms_vtPickRayOrig, ms_vtPickRayDir,
                              posVertices[sc_awFillRectIndices[i * 3 + 0]],
                              posVertices[sc_awFillRectIndices[i * 3 + 1]],
                              posVertices[sc_awFillRectIndices[i * 3 + 2]],
                              &u, &v, &t))
        {
            *px = ms_vtPickRayOrig.x + ms_vtPickRayDir.x * t;
            *py = ms_vtPickRayOrig.y + ms_vtPickRayDir.y * t;
            return true;
        }
    }

    return false;
}

bool CScreen::GetCursorZPosition(float* pz)
{
    D3DXVECTOR3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();

    constexpr float kExtent = 90000000.0f;
    std::array<TPosition, 4> posVertices{ {
        TPosition(v3Eye.x - kExtent, 0.0f, v3Eye.z + kExtent),
        TPosition(v3Eye.x - kExtent, 0.0f, v3Eye.z - kExtent),
        TPosition(v3Eye.x + kExtent, 0.0f, v3Eye.z + kExtent),
        TPosition(v3Eye.x + kExtent, 0.0f, v3Eye.z - kExtent),
    } };

    static const WORD sc_awFillRectIndices[6] = { 0, 2, 1, 2, 3, 1, };

    float u, v, t;

    for (int i = 0; i < 2; ++i)
    {
        if (IntersectTriangle(ms_vtPickRayOrig, ms_vtPickRayDir,
                              posVertices[sc_awFillRectIndices[i * 3 + 0]],
                              posVertices[sc_awFillRectIndices[i * 3 + 1]],
                              posVertices[sc_awFillRectIndices[i * 3 + 2]],
                              &u, &v, &t))
        {
            *pz = ms_vtPickRayOrig.z + ms_vtPickRayDir.z * t;
            return true;
        }
    }

    return false;
}

void CScreen::SetDiffuseColor(DWORD diffuseColor)
{
    ms_diffuseColor = diffuseColor;
}

void CScreen::SetDiffuseColor(float r, float g, float b, float a)
{
    ms_diffuseColor = GetColor(r, g, b, a);
}

void CScreen::SetClearColor(float r, float g, float b, float a)
{
    ms_clearColor = GetColor(r, g, b, a);
}

void CScreen::ClearDepthBuffer()
{
    assert(ms_lpd3dDevice != NULL);
    ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_ZBUFFER, ms_clearColor, ms_clearDepth, ms_clearStencil);
}

void CScreen::Clear()
{
    assert(ms_lpd3dDevice != NULL);
    ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, ms_clearColor, ms_clearDepth, ms_clearStencil);
}

BOOL CScreen::IsLostDevice()
{
    if (!ms_lpd3dDevice)
    {
        return TRUE;
    }

    IDirect3DDevice9Ex & rkD3DDev = *ms_lpd3dDevice;
    HRESULT hrTestCooperativeLevel = rkD3DDev.TestCooperativeLevel();

    if (FAILED(hrTestCooperativeLevel))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL CScreen::RestoreDevice()
{
    if (!ms_lpd3dDevice)
    {
        return FALSE;
    }

    UINT iD3DAdapterInfo = ms_iD3DAdapterInfo;
    IDirect3D9Ex & rkD3D = *ms_lpd3d;
    IDirect3DDevice9Ex & rkD3DDev = *ms_lpd3dDevice;
    D3DPRESENT_PARAMETERS & rkD3DPP = ms_d3dPresentParameter;
    D3D_CDisplayModeAutoDetector & rkD3DDetector = ms_kD3DDetector;

    HRESULT hrTestCooperativeLevel = rkD3DDev.TestCooperativeLevel();

    if (FAILED(hrTestCooperativeLevel))
    {
        if (D3DERR_DEVICELOST == hrTestCooperativeLevel)
        {
            return FALSE;
        }

        if (D3DERR_DEVICENOTRESET == hrTestCooperativeLevel)
        {
            D3D_CAdapterInfo* pkD3DAdapterInfo = rkD3DDetector.GetD3DAdapterInfop(ms_iD3DAdapterInfo);

            if (!pkD3DAdapterInfo)
            {
                return FALSE;
            }

            D3DDISPLAYMODEEX d3dDisplayModeEx = {};
            d3dDisplayModeEx.Size = sizeof(D3DDISPLAYMODEEX);

            if (FAILED(rkD3D.GetAdapterDisplayModeEx(iD3DAdapterInfo, &d3dDisplayModeEx, NULL)))
            {
                return FALSE;
            }

            rkD3DPP.BackBufferFormat = d3dDisplayModeEx.Format;

            /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
            ImGui_ImplDX9_InvalidateDeviceObjects();
#endif
            /* ----------------------------------------------------- */

            HRESULT hrReset = rkD3DDev.Reset(&rkD3DPP);

            if (FAILED(hrReset))
            {
                return FALSE;
            }

            STATEMANAGER.SetDefaultState();

            /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
            ImGui_ImplDX9_CreateDeviceObjects();
#endif
            /* ----------------------------------------------------- */
        }
    }

    return TRUE;

}

bool CScreen::Begin()
{
    assert(ms_lpd3dDevice != NULL);
    ResetFaceCount();

    if (!STATEMANAGER.BeginScene())
    {
        Tracenf("BeginScene FAILED\n");
        return false;
    }

    return true;
}

void CScreen::End()
{
    STATEMANAGER.EndScene();
}

extern bool g_isBrowserMode;
extern RECT g_rcBrowser;

void CScreen::Show(HWND hWnd)
{
    assert(ms_lpd3dDevice != NULL);

    if (g_isBrowserMode)
    {
        RECT rcTop = { static_cast<long>(0), static_cast<long>(0), static_cast<long>(ms_d3dPresentParameter.BackBufferWidth), static_cast<long>(g_rcBrowser.top)};
        RECT rcBottom = {0, g_rcBrowser.bottom, static_cast<long>(ms_d3dPresentParameter.BackBufferWidth), static_cast<long>(ms_d3dPresentParameter.BackBufferHeight)};
        RECT rcLeft = {0, g_rcBrowser.top, g_rcBrowser.left, g_rcBrowser.bottom};
        RECT rcRight = {g_rcBrowser.right, g_rcBrowser.top, static_cast<long>(ms_d3dPresentParameter.BackBufferWidth), g_rcBrowser.bottom};

        ms_lpd3dDevice->Present(&rcTop, &rcTop, hWnd, NULL);
        ms_lpd3dDevice->Present(&rcBottom, &rcBottom, hWnd, NULL);
        ms_lpd3dDevice->Present(&rcLeft, &rcLeft, hWnd, NULL);
        ms_lpd3dDevice->Present(&rcRight, &rcRight, hWnd, NULL);
    }

    else
    {
        HRESULT hr = ms_lpd3dDevice->Present(NULL, NULL, hWnd, NULL);

        if (D3DERR_DEVICELOST == hr)
        {
            RestoreDevice();
        }
    }
}

void CScreen::Show(RECT * pSrcRect)
{
    assert(ms_lpd3dDevice != NULL);
    ms_lpd3dDevice->Present(pSrcRect, NULL, NULL, NULL);
}

void CScreen::Show(RECT * pSrcRect, HWND hWnd)
{
    assert(ms_lpd3dDevice != NULL);
    ms_lpd3dDevice->Present(pSrcRect, NULL, hWnd, NULL);
}

void CScreen::ProjectPosition(float x, float y, float z, float* pfX, float* pfY)
{
    D3DXVECTOR3 Input(x, y, z);
    D3DXVECTOR3 Output;
    D3DXVec3Project(&Output, &Input, &ms_Viewport, &ms_matProj, &ms_matView, &ms_matWorld);

    *pfX = Output.x;
    *pfY = Output.y;
}

void CScreen::ProjectPosition(float x, float y, float z, float* pfX, float* pfY, float* pfZ)
{
    D3DXVECTOR3 Input(x, y, z);
    D3DXVECTOR3 Output;
    D3DXVec3Project(&Output, &Input, &ms_Viewport, &ms_matProj, &ms_matView, &ms_matWorld);

    *pfX = Output.x;
    *pfY = Output.y;
    *pfZ = Output.z;
}

CScreen::CScreen()
{
}

CScreen::~CScreen()
{
}

void CScreen::BuildViewFrustum()
{
    const D3DXVECTOR3& c_rv3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();
    const D3DXVECTOR3& c_rv3View = CCameraManager::Instance().GetCurrentCamera()->GetView();
    auto vv = ms_matView * ms_matProj;
    ms_frustum.BuildViewFrustum2(
        vv,
        ms_fNearY,
        ms_fFarY,
        ms_fFieldOfView,
        ms_fAspect,
        c_rv3Eye, c_rv3View);
}
