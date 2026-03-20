#include "StdAfx.h"
#include "GrpDevice.h"
#include "../EterBase/Stl.h"
#include "../EterBase/Debug.h"

/* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
#include "ImGui/imgui_impl_dx9.h"
#endif
/* ----------------------------------------------------- */

/* - SHADER -------------------------------------------- */
#include "ShaderVertexDeclarations.h"
#include "ShaderKeys.h"
#include <cstring>
/* ----------------------------------------------------- */

D3DPRESENT_PARAMETERS g_kD3DPP;
bool g_isBrowserMode = false;
RECT g_rcBrowser;

CGraphicDevice::CGraphicDevice()
    : m_uBackBufferCount(0)
{
    __Initialize();
}

CGraphicDevice::~CGraphicDevice()
{
    Destroy();
}

void CGraphicDevice::__Initialize()
{
    ms_iD3DAdapterInfo = D3DADAPTER_DEFAULT;
    ms_iD3DDevInfo = D3DADAPTER_DEFAULT;
    ms_iD3DModeInfo = D3DADAPTER_DEFAULT;

    ms_lpd3d			= NULL;
    ms_lpd3dDevice		= NULL;
    ms_lpd3dMatStack	= NULL;

    ms_dwWavingEndTime = 0;
    ms_dwFlashingEndTime = 0;

    m_pStateManager		= NULL;

    __InitializeDefaultIndexBufferList();
    __InitializePDTVertexBufferList();
}

void CGraphicDevice::RegisterWarningString(UINT uiMsg, const char* c_szString)
{
    m_kMap_strWarningMessage[uiMsg] = c_szString;
}

void CGraphicDevice::__WarningMessage(HWND hWnd, UINT uiMsg)
{
    if (!m_kMap_strWarningMessage.contains(uiMsg))
    {
        return;
    }

    MessageBox(hWnd, m_kMap_strWarningMessage[uiMsg].c_str(), "Warning", MB_OK | MB_TOPMOST);
}

void CGraphicDevice::MoveWebBrowserRect(const RECT& c_rcWebPage)
{
    g_rcBrowser = c_rcWebPage;
}

void CGraphicDevice::EnableWebBrowserMode(const RECT& c_rcWebPage)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    D3DPRESENT_PARAMETERS& rkD3DPP = ms_d3dPresentParameter;

    g_isBrowserMode = true;

    if (D3DSWAPEFFECT_COPY == rkD3DPP.SwapEffect)
    {
        return;
    }

    g_kD3DPP = rkD3DPP;
    g_rcBrowser = c_rcWebPage;

    //rkD3DPP.Windowed=TRUE;
    rkD3DPP.SwapEffect = D3DSWAPEFFECT_COPY;
    rkD3DPP.BackBufferCount = 1;
    rkD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
    ImGui_ImplDX9_InvalidateDeviceObjects();
#endif
    /* ----------------------------------------------------- */

    /* - SHADER [RESOURCES] -------------------------------- */
    RegisterShaderProvider(nullptr);
    __DestroyShaderResources();
    /* ----------------------------------------------------- */

    IDirect3DDevice9Ex& rkD3DDev = *ms_lpd3dDevice;
    HRESULT hr = rkD3DDev.Reset(&rkD3DPP);

    if (FAILED(hr))
    {
        return;
    }

    STATEMANAGER.SetDefaultState();

    /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
    ImGui_ImplDX9_CreateDeviceObjects();
#endif
    /* ----------------------------------------------------- */

    /* - SHADER [RESOURCES] -------------------------------- */
    if (__CreateShaderResources())
    {
        RegisterShaderProvider(this);
    }
    /* ----------------------------------------------------- */
}

void CGraphicDevice::DisableWebBrowserMode()
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    D3DPRESENT_PARAMETERS& rkD3DPP = ms_d3dPresentParameter;

    g_isBrowserMode = false;

    rkD3DPP = g_kD3DPP;

    /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
    ImGui_ImplDX9_InvalidateDeviceObjects();
#endif
    /* ----------------------------------------------------- */

    /* - SHADER [RESOURCES] -------------------------------- */
    RegisterShaderProvider(nullptr);
    __DestroyShaderResources();
    /* ----------------------------------------------------- */

    IDirect3DDevice9Ex& rkD3DDev = *ms_lpd3dDevice;
    HRESULT hr = rkD3DDev.Reset(&rkD3DPP);

    if (FAILED(hr))
    {
        return;
    }

    STATEMANAGER.SetDefaultState();

    /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
    ImGui_ImplDX9_CreateDeviceObjects();
#endif
    /* ----------------------------------------------------- */

    /* - SHADER [RESOURCES] -------------------------------- */
    if (__CreateShaderResources())
    {
        RegisterShaderProvider(this);
    }
    /* ----------------------------------------------------- */
}

bool CGraphicDevice::ResizeBackBuffer(UINT uWidth, UINT uHeight)
{
    if (!ms_lpd3dDevice)
    {
        return false;
    }

    D3DPRESENT_PARAMETERS& rkD3DPP = ms_d3dPresentParameter;

    if (rkD3DPP.Windowed)
    {
        if (rkD3DPP.BackBufferWidth != uWidth || rkD3DPP.BackBufferHeight != uHeight)
        {
            rkD3DPP.BackBufferWidth = uWidth;
            rkD3DPP.BackBufferHeight = uHeight;

            /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
            ImGui_ImplDX9_InvalidateDeviceObjects();
#endif
            /* ----------------------------------------------------- */

            /* - SHADER [RESOURCES] -------------------------------- */
            RegisterShaderProvider(nullptr);
            __DestroyShaderResources();
            /* ----------------------------------------------------- */

            IDirect3DDevice9Ex& rkD3DDev = *ms_lpd3dDevice;

            HRESULT hr = rkD3DDev.Reset(&rkD3DPP);

            if (FAILED(hr))
            {
                return false;
            }

            STATEMANAGER.SetDefaultState();

            /* - IMGUI --------------------------------------------- */
#ifdef _DEBUG
            ImGui_ImplDX9_CreateDeviceObjects();
#endif
            /* ----------------------------------------------------- */

            /* - SHADER [RESOURCES] -------------------------------- */
            if (__CreateShaderResources())
            {
                RegisterShaderProvider(this);
            }
            /* ----------------------------------------------------- */
        }
    }

    return true;
}

void CGraphicDevice::UploadWaterConstants(const WaterShaderInputs& inputs)
{
    if (!ms_lpd3dDevice || !inputs.settings)
    {
        return;
    }

    const WaterShaderSettings& ws = *inputs.settings;
    using spwd = ShaderDefaults::Water::Displacement;

    // ---- Build CBs (packing layer owns layout) ----
    WaterPerFrameCB perFrame{};
    WaterDisplacementCB disp{};
    WaterMaterialCB material{};
    WaterMatricesCB mats{};

    // per-frame (VS c0..c1, PS c0)
    perFrame.slot0 = {
        inputs.vs.perFrame.slot0[0], // WindDirection U
        inputs.vs.perFrame.slot0[1], // WindDirection V
        inputs.vs.perFrame.slot0[2], // WindStrength
        inputs.vs.perFrame.slot0[3]  // TimeSeconds
    };
    perFrame.slot1 = {
        inputs.vs.perFrame.slot1[0], // CameraPos x
        inputs.vs.perFrame.slot1[1], // CameraPos y
        inputs.vs.perFrame.slot1[2], // CameraPos z
        0.0f
    };
    // displacement (VS c2..c6)
    disp.slot0 = {
        ws.displacement.heightIntensity,
        ws.basic.worldUVScale,
        ws.displacement.waveTiling,
        ws.displacement.waveSteepness
    };
    disp.slot1 = ws.displacement.waveAmplitude;
    disp.slot2 = ws.displacement.wavesIntensity;
    disp.slot3 = ws.displacement.wavesNoise;
    disp.slot4 = {
        ws.displacement.waveAmplitudeFactor,
        spwd::HeightAmplitude,
        0.0f,
        0.0f
    };

    // material (PS c1..c4)
    material.slot0 = {
        ws.reflection.specularIntensity[0],
        ws.reflection.specularIntensity[1],
        ws.reflection.specularIntensity[2],
        ws.reflection.specularIntensity[3]
    };
    material.slot1 = {
        inputs.ps.material.slot1[0], // LightDir x
        inputs.ps.material.slot1[1], // LightDir y
        inputs.ps.material.slot1[2], // LightDir z
        0.0f
    };
    material.slot2 = {
        inputs.ps.material.slot2[0], // LightColor x
        inputs.ps.material.slot2[1], // LightColor y
        inputs.ps.material.slot2[2], // LightColor z
        1.0f
    };
    material.slot3 = {
        ws.basic.normalIntensity,
        ws.features.enableBlinnPhong ? 1.0f : 0.0f,
        static_cast<float>(ws.features.debugView),
        ws.refraction.waterClarity
    };
    material.slot4 = {
        ws.basic.surfaceColor[0],
        ws.basic.surfaceColor[1],
        ws.basic.surfaceColor[2],
        1.0f
    };

    material.slot5 = {
        ws.basic.deepColor[0],
        ws.basic.deepColor[1],
        ws.basic.deepColor[2],
        1.0f
    };

    // ---- Upload ----
    UploadVSConstants(0, perFrame.slot0.data(), 2);         // c0..c1
    UploadVSConstants(2, disp.slot0.data(), 5);             // c2..c6
    UploadVSConstants(7, inputs.vs.matrices.viewProj.data(), 4);  // c7..c10
    UploadVSConstants(11, inputs.vs.matrices.world.data(), 4);    // c11..c14
    UploadVSConstants(15, inputs.vs.matrices.view.data(), 4);     // c15..c18
    UploadVSConstants(19, inputs.vs.matrices.texTransform.data(), 4); // c19..c22

    UploadPSConstants(0, perFrame.slot1.data(), 1);         // c0
    UploadPSConstants(1, material.slot0.data(), 6);         // c1..c6
}

void CGraphicDevice::UploadSkyboxConstants(const SkyboxShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7
}

void CGraphicDevice::UploadCloudConstants(const CloudShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    CloudVSCB vs{};

    vs.uvScaleSpeed = {
        inputs.vs.uvScaleSpeed[0],
        inputs.vs.uvScaleSpeed[1],
        inputs.vs.uvScaleSpeed[2],
        inputs.vs.uvScaleSpeed[3]
    };

    vs.timeSeconds = {
        inputs.vs.timeSeconds[0],
        0.0f,
        0.0f,
        0.0f
    };

    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7
    UploadVSConstants(8, vs.uvScaleSpeed.data(), 1);
    UploadVSConstants(9, vs.timeSeconds.data(), 1);

    // --- Pack PS ---
    CloudPSCB ps{};
    ps.cloudTint = inputs.ps.cloudTint;

    UploadPSConstants(0, ps.cloudTint.data(), 1);     // PS c0
}

void CGraphicDevice::UploadWeaponTraceConstants(const WeaponTraceShaderInputs& inputs)
{
    WeaponTracePSCB ps{};

    ps.slot0 = { inputs.ps.slot0 };

    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7

    UploadPSConstants(0, ps.slot0.data(), 1);         // c0
}

void CGraphicDevice::UploadScreenPrimitiveConstants(const ScreenPrimitiveShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
        return;

    // --- Pack ---
    ScreenPrimitivePSCB ps{};
    ps.mode = { inputs.ps.mode };
    ps.colorFactor = { inputs.ps.colorFactor };

    // --- Upload ---
    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7

    UploadPSConstants(0, ps.mode.data(), 2);          // PS c0..c1
}

void CGraphicDevice::UploadMiniMapConstants(const MiniMapShaderInputs& in)
{
    if (!ms_lpd3dDevice)
        return;

    // --- Pack PS ---
    MiniMapPSCB ps{};
    ps.colorFactor = in.ps.colorFactor;
    ps.flags = {
        in.ps.flags[0], // UseTexture
        in.ps.flags[1], // UseMask
        0.0f, 0.0f};

    // --- Upload ---
    UploadVSConstants(0, in.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, in.vs.world.data(), 4);    // c4..c7
    UploadVSConstants(8, in.vs.texTransform.data(), 4);   // c8뻙11

    UploadPSConstants(0, ps.colorFactor.data(), 2);    // c0뻙1
}

void CGraphicDevice::UploadTextConstants(const TextShaderInputs& in)
{
    std::array<float, 4> invScreenSize =
    {
        in.invScreenW,
        in.invScreenH,
        0.0f,
        0.0f
    };

    UploadVSConstants(0, invScreenSize.data(), 1); // c0
}

void CGraphicDevice::UploadEffectParticleConstants(const EffectParticleShaderInputs& in)
{
    if (!ms_lpd3dDevice)
        return;

    // --- Pack PS ---
    EffectParticlePSCB ps{};
    ps.textureFactor = in.ps.textureFactor;
    ps.ops = in.ps.ops;

    // --- Upload ---
    UploadVSConstants(0, in.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, in.vs.world.data(), 4);    // c4..c7
    UploadPSConstants(0, ps.textureFactor.data(), 2);     // PS c0..c1 (textureFactor + ops)
}

void CGraphicDevice::UploadEffectMeshConstants(const EffectMeshShaderInputs& in)
{
    if (!ms_lpd3dDevice)
        return;

    // --- Pack ---
    EffectMeshPSCB ps{};
    ps.textureFactor = in.ps.textureFactor;

    // --- Upload ---
    UploadVSConstants(0, in.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, in.vs.world.data(), 4);    // c4..c7

    UploadPSConstants(0, ps.textureFactor.data(), 1);   // PS c0
}

void CGraphicDevice::UploadModelConstants(const ModelShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    ModelPSCB ps{};
    ps.textureFlags = inputs.ps.textureFlags;

    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7

    UploadPSConstants(0, ps.textureFlags.data(), 1);    // PS c0
}

void CGraphicDevice::UploadDungeonConstants(const DungeonShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7
}

void CGraphicDevice::UploadSnowParticleConstants(const SnowParticleShaderInputs& in)
{
    if (!ms_lpd3dDevice)
        return;

    UploadVSConstants(0, in.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, in.vs.world.data(), 4);    // c4..c7
}

void CGraphicDevice::UploadTerrainConstants(const TerrainShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    // --- Pack ---
    TerrainPSCB ps{};

    ps.layerState = inputs.ps.layerState;

    // --- Upload ---
    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7
    UploadVSConstants(8, inputs.vs.colorTexMatrix.data(), 4);    // VS c8..c11
    UploadVSConstants(12, inputs.vs.alphaTexMatrix.data(), 4);    // VS c12..c15

    UploadPSConstants(0, ps.layerState.data(), 1);        // PS c0
}

void CGraphicDevice::UploadTerrainMarkedAreaConstants(const TerrainMarkedAreaShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    TerrainMarkedAreaPSCB ps{};

    ps.alpha = inputs.ps.alpha;

    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7
    UploadVSConstants(8, inputs.vs.viewInverse.data(), 4);     // VS c8..c11
    UploadVSConstants(12, inputs.vs.texScale.data(), 1);        // VS c12

    UploadPSConstants(0, ps.alpha.data(), 1);           // PS c0
}

void CGraphicDevice::UploadFlyTraceConstants(const FlyTraceShaderInputs& inputs)
{
    if (!ms_lpd3dDevice)
    {
        return;
    }

    UploadVSConstants(0, inputs.vs.viewProj.data(), 4); // c0..c3
    UploadVSConstants(4, inputs.vs.world.data(), 4);    // c4..c7
}

void CGraphicDevice::UploadVSConstants(UINT startRegister, const float* data, UINT registerCount)
{
    if (!ms_lpd3dDevice || !data || registerCount == 0)
    {
        return;
    }

    ms_lpd3dDevice->SetVertexShaderConstantF(startRegister, data, registerCount);
}

void CGraphicDevice::UploadPSConstants(UINT startRegister, const float* data, UINT registerCount)
{
    if (!ms_lpd3dDevice || !data || registerCount == 0)
    {
        return;
    }

    ms_lpd3dDevice->SetPixelShaderConstantF(startRegister, data, registerCount);
}

void CGraphicDevice::SetFrameShaderInputs(const FrameShaderInputs& in)
{
    m_frameShaderInputs = in;
}

const FrameShaderInputs& CGraphicDevice::GetFrameShaderInputs() const
{
    return m_frameShaderInputs;
}

void CGraphicDevice::UploadFrameConstants(const FrameShaderInputs& frame)
{
    float v0[4] = { frame.cameraPos[0], frame.cameraPos[1], frame.cameraPos[2], 1.0f };
    float v1[4] = { frame.sunDir[0], frame.sunDir[1], frame.sunDir[2], 0.0f };
    float v2[4] = { frame.sunColor[0], frame.sunColor[1], frame.sunColor[2], 0.0f };
    float v3[4] = { frame.ambientColor[0], frame.ambientColor[1], frame.ambientColor[2], 0.0f };

    UploadPSConstants(1, v0, 1);
    UploadPSConstants(2, v1, 1);
    UploadPSConstants(3, v2, 1);
    UploadPSConstants(4, v3, 1);
}

void CGraphicDevice::ComputeWorldViewProj(const D3DXMATRIX& world,
    D3DXMATRIX& outWVP) const
{
    const D3DXMATRIX& view = CGraphicBase::GetViewMatrix();
    const D3DXMATRIX& proj = CGraphicBase::GetProjMatrix();

    outWVP = world * view * proj;
}

void CGraphicDevice::FillScreenPrimitive3D(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const
{
    // world
    std::memcpy(out.vs.world.data(), &world, sizeof(D3DXMATRIX));

    // viewProj
    D3DXMATRIX viewProj = CGraphicBase::GetViewMatrix() * CGraphicBase::GetProjMatrix();
    std::memcpy(out.vs.viewProj.data(), &viewProj, sizeof(D3DXMATRIX));
}

void CGraphicDevice::FillScreenPrimitive2D(ScreenPrimitiveShaderInputs& out) const
{
    // world = identity
    const D3DXMATRIX& identity = CGraphicBase::GetIdentityMatrix();
    std::memcpy(out.vs.world.data(), &identity, sizeof(D3DXMATRIX));

    // viewProj = proj (no view)
    const D3DXMATRIX& proj = CGraphicBase::GetProjMatrix();
    std::memcpy(out.vs.viewProj.data(), &proj, sizeof(D3DXMATRIX));
}

void CGraphicDevice::FillScreenPrimitive2DWorld(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const
{
    // world
    std::memcpy(out.vs.world.data(), &world, sizeof(D3DXMATRIX));

    // viewProj = proj
    const D3DXMATRIX& proj = CGraphicBase::GetProjMatrix();
    std::memcpy(out.vs.viewProj.data(), &proj, sizeof(D3DXMATRIX));
}

void CGraphicDevice::FillScreenPrimitive2DOrtho01World(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const
{
    // world
    std::memcpy(out.vs.world.data(), &world, sizeof(D3DXMATRIX));

    // viewProj = ortho
    D3DXMATRIX proj;
    D3DXMatrixOrthoOffCenterRH(&proj, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);

    std::memcpy(out.vs.viewProj.data(), &proj, sizeof(D3DXMATRIX));
}

void CGraphicDevice::FillScreenPrimitive2DOrthoPixel(float width, float height, ScreenPrimitiveShaderInputs& out) const
{
    // world = identity
    const D3DXMATRIX& identity = CGraphicBase::GetIdentityMatrix();
    std::memcpy(out.vs.world.data(), &identity, sizeof(D3DXMATRIX));

    // viewProj = ortho
    D3DXMATRIX proj;
    D3DXMatrixOrthoOffCenterRH(&proj,
        0.0f, width,
        height, 0.0f,
        -1.0f, 1.0f);

    std::memcpy(out.vs.viewProj.data(), &proj, sizeof(D3DXMATRIX));
}

bool CGraphicDevice::BindShader(ShaderID id) const
{
    if (!ms_lpd3dDevice)
        return false;

    assert(static_cast<size_t>(id) < m_shaders.size());
    const auto index = static_cast<size_t>(std::to_underlying(id));
    const auto& program = m_shaders[index];

    if (!program.vs.shader || !program.ps.shader)
        return false;

    STATEMANAGER.SetVertexShader(program.vs.shader);
    STATEMANAGER.SetPixelShader(program.ps.shader);

    if (program.layout)
    {
        STATEMANAGER.SetVertexDeclaration(program.layout);
    }

    const IShaderProvider* sp = GetShaderProvider();
    if (sp)
    {
        UploadFrameConstants(sp->GetFrameShaderInputs());
    }

    return true;
}

void CGraphicDevice::BindDepthState(EDepthState state) const
{
    switch (state)
    {
    case EDepthState::EnabledWrite:
        STATEMANAGER.SetRenderState(D3DRS_ZENABLE, TRUE);
        STATEMANAGER.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        break;

    case EDepthState::EnabledReadOnly:
        STATEMANAGER.SetRenderState(D3DRS_ZENABLE, TRUE);
        STATEMANAGER.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        break;

    case EDepthState::Disabled:
        STATEMANAGER.SetRenderState(D3DRS_ZENABLE, FALSE);
        STATEMANAGER.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        break;
    }
}

void CGraphicDevice::BindBlendState(EBlendState state) const
{
    switch (state)
    {
    case EBlendState::Opaque:
        STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        break;

    case EBlendState::AlphaBlend:
        STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        break;

    case EBlendState::AlphaAdditive:
        STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;

    case EBlendState::Additive:
        STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        break;

    case EBlendState::One_InvSrcColor:
        STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
        break;

    case EBlendState::Zero_SrcColor:
        STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
        break;
    }
}

void CGraphicDevice::BindRasterState(ERasterState state) const
{
    switch (state)
    {
    case ERasterState::CullBack:
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        break;

    case ERasterState::CullFront:
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
        STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        break;

    case ERasterState::CullNone:
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        break;

    case ERasterState::Wireframe:
        STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
        break;
    }
}

void CGraphicDevice::BindSamplerState(UINT slot, ESamplerState state) const
{
    switch (state)
    {
    case ESamplerState::LinearClamp:
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        break;

    case ESamplerState::LinearWrap:
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        break;

    case ESamplerState::PointClamp:
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        STATEMANAGER.SetSamplerState(slot, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        break;
    }
}

bool CGraphicDevice::BindPipelineState(const PipelineStateDesc& desc) const
{
    if (!BindShader(desc.shader))
        return false;

    BindDepthState(desc.depth);
    BindBlendState(desc.blend);
    BindRasterState(desc.raster);

    for (UINT i = 0; i < desc.samplerCount; ++i)
    {
        BindSamplerState(desc.samplers[i].slot, desc.samplers[i].state);
    }

    return true;
}

CGraphicDevice::EDeviceState CGraphicDevice::GetDeviceState()
{
    if (!ms_lpd3dDevice)
    {
        return DEVICESTATE_NULL;
    }

    HRESULT hr;

    if (FAILED(hr = ms_lpd3dDevice->TestCooperativeLevel()))
    {
        if (D3DERR_DEVICELOST == hr)
        {
            return DEVICESTATE_BROKEN;
        }

        if (D3DERR_DEVICENOTRESET == hr)
        {
            return DEVICESTATE_NEEDS_RESET;
        }

        return DEVICESTATE_BROKEN;
    }

    return DEVICESTATE_OK;
}

bool CGraphicDevice::Reset()
{
    D3DDISPLAYMODEEX fullscreenMode = {};
    D3DDISPLAYMODEEX* pFullscreenMode = nullptr;

    if (!ms_d3dPresentParameter.Windowed)
    {
        fullscreenMode.Size = sizeof(D3DDISPLAYMODEEX);
        fullscreenMode.Width = ms_d3dPresentParameter.BackBufferWidth;
        fullscreenMode.Height = ms_d3dPresentParameter.BackBufferHeight;
        fullscreenMode.Format = ms_d3dPresentParameter.BackBufferFormat;
        fullscreenMode.RefreshRate = ms_d3dPresentParameter.FullScreen_RefreshRateInHz;
        fullscreenMode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;

        pFullscreenMode = &fullscreenMode;
    }

    const HRESULT hr = ms_lpd3dDevice->ResetEx(&ms_d3dPresentParameter, pFullscreenMode);
    return SUCCEEDED(hr);
}


static LPDIRECT3DSURFACE9 s_lpStencil;
static DWORD   s_MaxTextureWidth, s_MaxTextureHeight;

BOOL EL3D_ConfirmDevice(D3DCAPS9& rkD3DCaps, UINT uBehavior, D3DFORMAT)
{
    if (rkD3DCaps.VertexShaderVersion < D3DVS_VERSION(3, 0))
        return FALSE;

    if (rkD3DCaps.PixelShaderVersion < D3DPS_VERSION(3, 0))
        return FALSE;

    s_MaxTextureWidth = rkD3DCaps.MaxTextureWidth;
    s_MaxTextureHeight = rkD3DCaps.MaxTextureHeight;

    return TRUE;
}

DWORD GetMaxTextureWidth()
{
    return s_MaxTextureWidth;
}

DWORD GetMaxTextureHeight()
{
    return s_MaxTextureHeight;
}

/* - SHADER [LIFETIME] ---------------------------------- */
bool CGraphicDevice::__CreateShaderResources()
{
    // Input layouts (device objects)
    if (!CShaderInputLayouts::CreateAll(ms_lpd3dDevice))
    {
        TraceError("Failed to create input layouts");
        return false;
    }

    ShaderManager& sm = ShaderManager::Instance();
    sm.SetDevice(ms_lpd3dDevice);

    using namespace ShaderKeys;

    struct ShaderDesc
    {
        ShaderID id;
        const char* vsPath;
        const char* psPath;
        EShaderInputLayout layout;
    };

    static constexpr std::array<ShaderDesc, static_cast<size_t>(std::to_underlying(ShaderID::Count))> kShaderTable =
    {
        ShaderDesc{ ShaderID::Water,             Water::VS,             Water::PS,             EShaderInputLayout::PTC },
        ShaderDesc{ ShaderID::SkyBox,            SkyBox::VS,            SkyBox::PS,            EShaderInputLayout::PCT },
        ShaderDesc{ ShaderID::Cloud,             Clouds::VS,            Clouds::PS,            EShaderInputLayout::PCT },
        ShaderDesc{ ShaderID::WeaponTrace,       WeaponTrace::VS,       WeaponTrace::PS,       EShaderInputLayout::PCT },
        ShaderDesc{ ShaderID::ScreenPrimitive,   ScreenPrimitive::VS,   ScreenPrimitive::PS,   EShaderInputLayout::PCT },
        ShaderDesc{ ShaderID::MiniMap,           MiniMap::VS,           MiniMap::PS,           EShaderInputLayout::PT },
        ShaderDesc{ ShaderID::Text,              Text::VS,              Text::PS,              EShaderInputLayout::PCT },
        ShaderDesc{ ShaderID::EffectParticle,    EffectParticle::VS,    EffectParticle::PS,    EShaderInputLayout::PT },
        ShaderDesc{ ShaderID::EffectMesh,        EffectMesh::VS,        EffectMesh::PS,        EShaderInputLayout::PT },
        ShaderDesc{ ShaderID::Model,             Model::VS,             Model::PS,             EShaderInputLayout::PNT },
        ShaderDesc{ ShaderID::Dungeon,           Dungeon::VS,           Dungeon::PS,           EShaderInputLayout::PNTT },
        ShaderDesc{ ShaderID::SnowParticle,      SnowParticle::VS,      SnowParticle::PS,      EShaderInputLayout::PT },
        ShaderDesc{ ShaderID::Terrain,           Terrain::VS,           Terrain::PS,           EShaderInputLayout::PN },
        ShaderDesc{ ShaderID::TerrainMarkedArea, TerrainMarkedArea::VS, TerrainMarkedArea::PS, EShaderInputLayout::PN },
        ShaderDesc{ ShaderID::FlyTrace,          FlyTrace::VS,          FlyTrace::PS,          EShaderInputLayout::PCT },
    };

    static_assert(kShaderTable.size() == static_cast<size_t>(std::to_underlying(ShaderID::Count)), "ShaderID enum and shader table are out of sync");

    for (const auto& desc : kShaderTable)
    {
        const auto index = static_cast<size_t>(std::to_underlying(desc.id));

        bool okVS = sm.GetVertexShaderFromPack(desc.vsPath, &m_shaders[index].vs);
        bool okPS = sm.GetPixelShaderFromPack(desc.psPath, &m_shaders[index].ps);

        if (!okVS || !okPS)
        {
            TraceError("Failed to load shader program");
            return false;
        }

        m_shaders[index].layout = CShaderInputLayouts::Get(desc.layout);
    }

    return true;
}

void CGraphicDevice::__DestroyShaderResources()
{
    // IMPORTANT:
    // ShaderManager owns/releases the COM objects. Our members are raw copies.
    // So clear the cache first, then null our copies.

    ShaderManager::Instance().Clear();

    for (auto& s : m_shaders)
        s = {};

    // Input layouts must be released on reset/destroy
    CShaderInputLayouts::DestroyAll();
}
/* ----------------------------------------------------- */


int CGraphicDevice::Create(HWND hWnd, int iHres, int iVres, bool Windowed, int iReflashRate)
{
    int iRet = CREATE_OK;

    Destroy();

    ms_iWidth	= iHres;
    ms_iHeight	= iVres;

    ms_hWnd		= hWnd;
    ms_hDC		= GetDC(hWnd);
    ms_lpd3d = nullptr;
    HRESULT hrEx = Direct3DCreate9Ex(D3D_SDK_VERSION, &ms_lpd3d);

    if (FAILED(hrEx) || !ms_lpd3d)
    {
        return CREATE_NO_DIRECTX;
    }

    if (!ms_kD3DDetector.Build(*ms_lpd3d, EL3D_ConfirmDevice))
    {
        return CREATE_ENUM;
    }

    if (!ms_kD3DDetector.Find(800, 600, &ms_iD3DModeInfo, &ms_iD3DDevInfo, &ms_iD3DAdapterInfo))
    {
        return CREATE_DETECT;
    }

    std::string stDevList;
    ms_kD3DDetector.GetString(&stDevList);

    //Tracen(stDevList.c_str());
    //Tracenf("adapter %d, device %d, mode %d", ms_iD3DAdapterInfo, ms_iD3DDevInfo, ms_iD3DModeInfo);

    D3D_CAdapterInfo * pkD3DAdapterInfo = ms_kD3DDetector.GetD3DAdapterInfop(ms_iD3DAdapterInfo);

    if (!pkD3DAdapterInfo)
    {
        Tracenf("adapter %d is EMPTY", ms_iD3DAdapterInfo);
        return CREATE_DETECT;
    }

    D3D_SModeInfo * pkD3DModeInfo = pkD3DAdapterInfo->GetD3DModeInfop(ms_iD3DDevInfo, ms_iD3DModeInfo);

    if (!pkD3DModeInfo)
    {
        Tracenf("device %d, mode %d is EMPTY", ms_iD3DDevInfo, ms_iD3DModeInfo);
        return CREATE_DETECT;
    }

    std::string stModeInfo;
    pkD3DModeInfo->GetString(&stModeInfo);

    //Tracen(stModeInfo.c_str());

    int ErrorCorrection = 0;

RETRY:
    ZeroMemory(&ms_d3dPresentParameter, sizeof(ms_d3dPresentParameter));

    ms_d3dPresentParameter.Windowed							= Windowed;
    ms_d3dPresentParameter.BackBufferWidth					= iHres;
    ms_d3dPresentParameter.BackBufferHeight					= iVres;
    ms_d3dPresentParameter.hDeviceWindow					= hWnd;
    ms_d3dPresentParameter.BackBufferCount					= m_uBackBufferCount;
    ms_d3dPresentParameter.SwapEffect						= D3DSWAPEFFECT_DISCARD;

    if (Windowed)
    {
        ms_d3dPresentParameter.BackBufferFormat				= pkD3DAdapterInfo->GetDesktopD3DDisplayModer().Format;
    }

    else
    {
        ms_d3dPresentParameter.BackBufferFormat				= pkD3DModeInfo->m_eD3DFmtPixel;
        ms_d3dPresentParameter.FullScreen_RefreshRateInHz	= iReflashRate;
    }

    ms_d3dPresentParameter.PresentationInterval             = D3DPRESENT_INTERVAL_IMMEDIATE;
    ms_d3dPresentParameter.Flags							= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    ms_d3dPresentParameter.EnableAutoDepthStencil			= TRUE;
    ms_d3dPresentParameter.AutoDepthStencilFormat			= pkD3DModeInfo->m_eD3DFmtDepthStencil;

    ms_dwD3DBehavior = pkD3DModeInfo->m_deviceFlags;

    D3DDISPLAYMODEEX fmEx;
    ZeroMemory(&fmEx, sizeof(fmEx));

    if (!Windowed)
    {
        fmEx.Size = sizeof(D3DDISPLAYMODEEX);
        fmEx.Width = iHres;
        fmEx.Height = iVres;
        fmEx.RefreshRate = iReflashRate;
        fmEx.Format = pkD3DModeInfo->m_eD3DFmtPixel;
        fmEx.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;
    }

    if (FAILED(ms_hLastResult = ms_lpd3d->CreateDeviceEx(
                                    ms_iD3DAdapterInfo,
                                    D3DDEVTYPE_HAL,
                                    hWnd,
                                    // 2004. 1. 9 myevan 버텍스 프로세싱 방식 자동 선택 추가
                                    pkD3DModeInfo->m_deviceFlags,
                                    &ms_d3dPresentParameter,
                                    Windowed ? nullptr : &fmEx,   // pass nullptr if windowed
                                    &ms_lpd3dDevice)))
    {
        switch (ms_hLastResult)
        {
            case D3DERR_INVALIDCALL:
                Tracen("IDirect3DDevice.CreateDeviceEx - ERROR D3DERR_INVALIDCALL\nThe method call is invalid. For example, a method's parameter may have an invalid value.");
                break;

            case D3DERR_NOTAVAILABLE:
                Tracen("IDirect3DDevice.CreateDeviceEx - ERROR D3DERR_NOTAVAILABLE\nThis device does not support the queried technique. ");
                break;

            case D3DERR_OUTOFVIDEOMEMORY:
                Tracen("IDirect3DDevice.CreateDeviceEx - ERROR D3DERR_OUTOFVIDEOMEMORY\nDirect3D does not have enough display memory to perform the operation");
                break;

            default:
                Tracenf("IDirect3DDevice.CreateDeviceEx - ERROR %d", ms_hLastResult);
                break;
        }

        if (ErrorCorrection)
        {
            return CREATE_DEVICE;
        }

        // 2004. 1. 9 myevan 큰의미 없는 코드인듯.. 에러나면 표시하고 종료하자
        iReflashRate = 0;
        ++ErrorCorrection;
        iRet = CREATE_REFRESHRATE;
        goto RETRY;
    }

    if (FAILED((ms_hLastResult = ms_lpd3dDevice->GetDeviceCaps(&ms_d3dCaps))))
    {
        Tracenf("IDirect3DDevice.GetDeviceCaps - ERROR %d", ms_hLastResult);
        return CREATE_GET_DEVICE_CAPS2;
    }

    if (!Windowed)
    {
        SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, iHres, iVres, SWP_SHOWWINDOW);
    }

    ms_lpd3dDevice->GetViewport(&ms_Viewport);

    m_pStateManager = new CStateManager(ms_lpd3dDevice);

    D3DXCreateMatrixStack(0, &ms_lpd3dMatStack);
    ms_lpd3dMatStack->LoadIdentity();

    /* - SHADER [RESOURCES] -------------------------------- */
    if (__CreateShaderResources())
    {
        RegisterShaderProvider(this);
    }
    else
    {
        return CREATE_DEVICE;
    }
    /* ----------------------------------------------------- */

    D3DXMatrixIdentity(&ms_matIdentity);
    D3DXMatrixIdentity(&ms_matView);
    D3DXMatrixIdentity(&ms_matProj);
    D3DXMatrixIdentity(&ms_matInverseView);
    D3DXMatrixIdentity(&ms_matInverseViewYAxis);
    D3DXMatrixIdentity(&ms_matScreen0);
    D3DXMatrixIdentity(&ms_matScreen1);
    D3DXMatrixIdentity(&ms_matScreen2);

    ms_matScreen0._11 = 1;
    ms_matScreen0._22 = -1;

    ms_matScreen1._41 = 1;
    ms_matScreen1._42 = 1;

    ms_matScreen2._11 = (float) iHres / 2;
    ms_matScreen2._22 = (float) iVres / 2;

    D3DXCreateSphere(ms_lpd3dDevice, 1.0f, 32, 32, &ms_lpSphereMesh, NULL);
    D3DXCreateCylinder(ms_lpd3dDevice, 1.0f, 1.0f, 1.0f, 8, 8, &ms_lpCylinderMesh, NULL);

    ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);

    if (!__CreateDefaultIndexBufferList())
    {
        return false;
    }

    if (!__CreatePDTVertexBufferList())
    {
        return false;
    }

    return (iRet);
}

void CGraphicDevice::__InitializePDTVertexBufferList()
{
    for (UINT i = 0; i < PDT_VERTEXBUFFER_NUM; ++i)
    {
        ms_alpd3dPDTVB[i] = NULL;
    }
}

void CGraphicDevice::__DestroyPDTVertexBufferList()
{
    for (UINT i = 0; i < PDT_VERTEXBUFFER_NUM; ++i)
    {
        if (ms_alpd3dPDTVB[i])
        {
            ms_alpd3dPDTVB[i]->Release();
            ms_alpd3dPDTVB[i] = NULL;
        }
    }
}

bool CGraphicDevice::__CreatePDTVertexBufferList()
{
    for (UINT i = 0; i < PDT_VERTEXBUFFER_NUM; ++i)
    {
        if (FAILED(
                    ms_lpd3dDevice->CreateVertexBuffer(
                        sizeof(TPDTVertex)*PDT_VERTEX_NUM,
                        D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
                        D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1,
                        D3DPOOL_DEFAULT,
                        &ms_alpd3dPDTVB[i], nullptr)
                ))
        {
            return false;
        }
    }

    return true;
}

void CGraphicDevice::__InitializeDefaultIndexBufferList()
{
    for (UINT i = 0; i < DEFAULT_IB_NUM; ++i)
    {
        ms_alpd3dDefIB[i] = NULL;
    }
}

void CGraphicDevice::__DestroyDefaultIndexBufferList()
{
    for (UINT i = 0; i < DEFAULT_IB_NUM; ++i)
        if (ms_alpd3dDefIB[i])
        {
            ms_alpd3dDefIB[i]->Release();
            ms_alpd3dDefIB[i] = NULL;
        }
}

bool CGraphicDevice::__CreateDefaultIndexBuffer(UINT eDefIB, UINT uIdxCount, const WORD* c_awIndices)
{
    assert(ms_alpd3dDefIB[eDefIB] == NULL);

    if (FAILED(
                ms_lpd3dDevice->CreateIndexBuffer(
                    sizeof(WORD)*uIdxCount,
                    D3DUSAGE_WRITEONLY,
                    D3DFMT_INDEX16,
                    D3DPOOL_DEFAULT,
                    &ms_alpd3dDefIB[eDefIB], nullptr)
            ))
    {
        return false;
    }

    WORD* dstIndices;

    if (FAILED(
                ms_alpd3dDefIB[eDefIB]->Lock(0, 0, (void**)&dstIndices, 0)
            ))
    {
        return false;
    }

    memcpy(dstIndices, c_awIndices, sizeof(WORD)*uIdxCount);

    ms_alpd3dDefIB[eDefIB]->Unlock();

    return true;
}

bool CGraphicDevice::__CreateDefaultIndexBufferList()
{
    static const WORD c_awLineIndices[2] = { 0, 1, };
    static const WORD c_awLineTriIndices[6] = { 0, 1, 0, 2, 1, 2, };
    static const WORD c_awLineRectIndices[8] = { 0, 1, 0, 2, 1, 3, 2, 3,};
    static const WORD c_awLineCubeIndices[24] =
    {
        0, 1, 0, 2, 1, 3, 2, 3,
        0, 4, 1, 5, 2, 6, 3, 7,
        4, 5, 4, 6, 5, 7, 6, 7,
    };
    static const WORD c_awFillTriIndices[3] = { 0, 1, 2, };
    static const WORD c_awFillRectIndices[6] = { 0, 2, 1, 2, 3, 1, };
    static const WORD c_awFillCubeIndices[36] =
    {
        0, 1, 2, 1, 3, 2,
        2, 0, 6, 0, 4, 6,
        0, 1, 4, 1, 5, 4,
        1, 3, 5, 3, 7, 5,
        3, 2, 7, 2, 6, 7,
        4, 5, 6, 5, 7, 6,
    };

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE, 2, c_awLineIndices))
    {
        return false;
    }

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_TRI, 6, c_awLineTriIndices))
    {
        return false;
    }

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_RECT, 8, c_awLineRectIndices))
    {
        return false;
    }

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_CUBE, 24, c_awLineCubeIndices))
    {
        return false;
    }

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_TRI, 3, c_awFillTriIndices))
    {
        return false;
    }

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_RECT, 6, c_awFillRectIndices))
    {
        return false;
    }

    if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_CUBE, 36, c_awFillCubeIndices))
    {
        return false;
    }

    return true;
}

void CGraphicDevice::InitBackBufferCount(UINT uBackBufferCount)
{
    m_uBackBufferCount = uBackBufferCount;
}

void CGraphicDevice::Destroy()
{
    /* - SHADER [RESOURCES] -------------------------------- */
    RegisterShaderProvider(nullptr);
    __DestroyShaderResources();
    /* ----------------------------------------------------- */

    __DestroyPDTVertexBufferList();
    __DestroyDefaultIndexBufferList();

    if (ms_hDC)
    {
        ReleaseDC(ms_hWnd, ms_hDC);
        ms_hDC = NULL;
    }

    safe_release(ms_lpSphereMesh);
    safe_release(ms_lpCylinderMesh);

    safe_release(ms_lpd3dMatStack);
    safe_release(ms_lpd3dDevice);
    safe_release(ms_lpd3d);

    if (m_pStateManager)
    {
        delete m_pStateManager;
        m_pStateManager = NULL;
    }

    __Initialize();
}
