#pragma once

#include "ShaderManager.h"
#include "ShaderParameters.h"

enum class ShaderID
{
    Water,
    SkyBox,
    Cloud,
    WeaponTrace,
    ScreenPrimitive,
    MiniMap,
    Text,
    EffectParticle,
    EffectMesh,
    Model,
    Dungeon,
    SnowParticle,
    Terrain,
    TerrainMarkedArea,
    FlyTrace,
    Count
};

enum class EDepthState
{
    EnabledWrite,
    EnabledReadOnly,
    Disabled
};

enum class EBlendState
{
    Opaque,
    AlphaBlend,
    AlphaAdditive,   // ALPHABLENDENABLE=TRUE, SRCBLEND=SRCALPHA, DESTBLEND=ONE
    Additive,        // ALPHABLENDENABLE=TRUE, SRCBLEND=ONE,      DESTBLEND=ONE
    One_InvSrcColor, // ALPHABLENDENABLE=TRUE, SRCBLEND=ONE,      DESTBLEND=INVSRCOLOR
    Zero_SrcColor    // ALPHABLENDENABLE=TRUE, SRCBLEND=ZERO,     DESTBLEND=SRCCOLOR
};

enum class ERasterState
{
    CullBack,
    CullFront,
    CullNone,
    Wireframe
};

enum class ESamplerState
{
    LinearClamp,
    LinearWrap,
    PointClamp
};

struct PipelineStateDesc
{
    ShaderID shader;

    EDepthState depth;
    EBlendState blend;
    ERasterState raster;

    struct SamplerBinding
    {
        UINT slot;
        ESamplerState state;
    };

    const SamplerBinding* samplers;
    UINT samplerCount;
};

struct IShaderProvider
{
    virtual bool BindShader(ShaderID id) const = 0;

    virtual const FrameShaderInputs& GetFrameShaderInputs() const = 0;
    virtual void SetFrameShaderInputs(const FrameShaderInputs& in) = 0;

    virtual void FillScreenPrimitive3D(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const = 0;
    virtual void FillScreenPrimitive2D(ScreenPrimitiveShaderInputs& out) const = 0;
    virtual void FillScreenPrimitive2DWorld(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const = 0;
    virtual void ComputeWorldViewProj(const D3DXMATRIX& world, D3DXMATRIX& outWVP) const = 0;
    virtual void FillScreenPrimitive2DOrtho01World(const D3DXMATRIX& world, ScreenPrimitiveShaderInputs& out) const = 0;
    virtual void FillScreenPrimitive2DOrthoPixel(float width, float height, ScreenPrimitiveShaderInputs& out) const = 0;

    virtual void BindDepthState(EDepthState state) const = 0;
    virtual void BindBlendState(EBlendState state) const = 0;
    virtual void BindRasterState(ERasterState state) const = 0;
    virtual void BindSamplerState(UINT slot, ESamplerState state) const = 0;
    virtual bool BindPipelineState(const PipelineStateDesc& desc) const = 0;

protected:
    ~IShaderProvider() = default;
};

// Public read-only accessor
const IShaderProvider* GetShaderProvider();

// Public mutable accessor (avoid const_cast in call sites)
IShaderProvider* GetShaderProviderMutable();

// Internal registration (document: CGraphicDevice only)
void RegisterShaderProvider(IShaderProvider* provider);
