#pragma once

#include <utility>
#include <array>

#include "GrpBase.h"
#include "GrpDetector.h"
#include "StateManager.h"

/* - SHADER -------------------------------------------- */
#include "ShaderManager.h"
#include "ShaderProvider.h"
/* ----------------------------------------------------- */
/* - SHADER CONSTANT PACKS ----------------------------- */
#include "ShaderParameters.h"
/* ----------------------------------------------------- */

class CGraphicDevice : public CGraphicBase, public IShaderProvider
{
public:
    enum EDeviceState
    {
        DEVICESTATE_OK,
        DEVICESTATE_BROKEN,
        DEVICESTATE_NEEDS_RESET,
        DEVICESTATE_NULL
    };

    enum ECreateReturnValues
    {
        CREATE_OK				= (1 << 0),
        CREATE_NO_DIRECTX		= (1 << 1),
        CREATE_GET_DEVICE_CAPS	= (1 << 2),
        CREATE_GET_DEVICE_CAPS2 = (1 << 3),
        CREATE_DEVICE			= (1 << 4),
        CREATE_REFRESHRATE		= (1 << 5),
        CREATE_ENUM				= (1 << 6), // 2003. 01. 09. myevan 모드 리스트 얻기 실패
        CREATE_DETECT			= (1 << 7) // 2003. 01. 09. myevan 모드 선택 실패
    };

    CGraphicDevice();
    virtual ~CGraphicDevice();

    void			InitBackBufferCount(UINT uBackBufferCount);

    void			Destroy();
    int				Create(HWND hWnd, int hres, int vres, bool Windowed = true, int ReflashRate = 0);

    EDeviceState	GetDeviceState();
    bool			Reset();

    void			EnableWebBrowserMode(const RECT& c_rcWebPage);
    void			DisableWebBrowserMode();
    void			MoveWebBrowserRect(const RECT& c_rcWebPage);

    bool			ResizeBackBuffer(UINT uWidth, UINT uHeight);
    void			RegisterWarningString(UINT uiMsg, const char* c_szString);

    /* - SHADER [CONSTANT UPLOAD] ------------------------- */
    static void UploadWaterConstants(const WaterShaderInputs& inputs);
    static void UploadSkyboxConstants(const SkyboxShaderInputs& inputs);
    static void UploadCloudConstants(const CloudShaderInputs& inputs);
    static void UploadLensFlareConstants(const LensFlareShaderInputs& inputs);
    static void UploadWeaponTraceConstants(const WeaponTraceShaderInputs& inputs);
    static void UploadScreenPrimitiveConstants(const ScreenPrimitiveShaderInputs& inputs);
    static void UploadMiniMapConstants(const MiniMapShaderInputs& inputs);
    static void UploadTextConstants(const TextShaderInputs& inputs);
    static void UploadEffectParticleConstants(const EffectParticleShaderInputs& in);
    static void UploadEffectMeshConstants(const EffectMeshShaderInputs& in);
    static void UploadModelConstants(const ModelShaderInputs& inputs);
    /* ---------------------------------------------------- */

    /* - SHADER [LOW-LEVEL CONSTANT UPLOAD] ------------------ */
    static void UploadVSConstants(UINT startRegister, const float* data, UINT registerCount);
    static void UploadPSConstants(UINT startRegister, const float* data, UINT registerCount);
    /* -------------------------------------------------------- */

protected:
    void __Initialize();
    void __WarningMessage(HWND hWnd, UINT uiMsg);

    /* - SHADER [LIFETIME] --------------------------------- */
    bool __CreateShaderResources();
    void __DestroyShaderResources();
    /* ----------------------------------------------------- */

    void __InitializeDefaultIndexBufferList();
    void __DestroyDefaultIndexBufferList();
    bool __CreateDefaultIndexBufferList();
    bool __CreateDefaultIndexBuffer(UINT eDefIB, UINT uIdxCount, const WORD* c_awIndices);

    void __InitializePDTVertexBufferList();
    void __DestroyPDTVertexBufferList();
    bool __CreatePDTVertexBufferList();

    LPDIRECT3DVERTEXDECLARATION9 CreatePNTStreamVertexShader();
    LPDIRECT3DVERTEXDECLARATION9 CreatePNT2StreamVertexShader();

protected:
    DWORD						m_uBackBufferCount;
    std::map<UINT, std::string>	m_kMap_strWarningMessage;
    CStateManager*				m_pStateManager;

    struct ShaderProgram
    {
        ShaderManager::VertexShaderHandle vs;
        ShaderManager::PixelShaderHandle ps;
        LPDIRECT3DVERTEXDECLARATION9 layout = nullptr;
    };

    std::array<ShaderProgram, static_cast<size_t>(std::to_underlying(ShaderID::Count))> m_shaders;

    FrameShaderInputs m_frameShaderInputs{};

public:
    void SetFrameShaderInputs(const FrameShaderInputs& in) override;
    const FrameShaderInputs& GetFrameShaderInputs() const override;

    void FillScreenPrimitive3D(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const override;
    void FillScreenPrimitive2D(ScreenPrimitiveShaderInputs& out) const override;
    void FillScreenPrimitive2DWorld(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const override;
    void ComputeWorldViewProj(const D3DXMATRIX& world, D3DXMATRIX& outWVP) const override;
    void FillScreenPrimitive2DOrtho01World(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const override;
    void FillScreenPrimitive2DOrthoPixel(float width, float height, ScreenPrimitiveShaderInputs& out) const override;

    bool BindShader(ShaderID id) const override;
    void BindDepthState(EDepthState state) const override;
    void BindBlendState(EBlendState state) const override;
    void BindRasterState(ERasterState state) const override;
    void BindSamplerState(UINT slot, ESamplerState state) const override;
    bool BindPipelineState(const PipelineStateDesc& desc) const override;
};
