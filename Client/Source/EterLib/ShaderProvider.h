#pragma once

#include "ShaderManager.h"
#include "ShaderParameters.h"

enum class ShaderID
{
    Water,
    SkyBox,
    Cloud,
    LensFlare,
    WeaponTrace,
    ScreenPrimitive,
    MiniMap,
    Text,
    Count
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

protected:
    ~IShaderProvider() = default;
};

// Public read-only accessor
const IShaderProvider* GetShaderProvider();

// Public mutable accessor (avoid const_cast in call sites)
IShaderProvider* GetShaderProviderMutable();

// Internal registration (document: CGraphicDevice only)
void RegisterShaderProvider(IShaderProvider* provider);
