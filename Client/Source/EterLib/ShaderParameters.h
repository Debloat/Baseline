#pragma once

#include <array>
#include "ShaderParametersDefaults.h"
#include "GrpImage.h"

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct WaterShaderSettings
{
    struct Features
    {
        using D = ShaderDefaults::Water::Features;
        bool enableDisplacement                   = D::Displacement;
        bool enableMeanSky                        = D::MeanSky;
        bool enableFiltering                      = D::Filtering;
        bool enableBlinnPhong                     = D::BlinnPhong;

        // (0 = Off) (1 = Normals) (2 = Fog factor) (3 = Height)
        int  debugView                            = D::DebugView;
    } features;

    struct Basic
    {
        using D = ShaderDefaults::Water::Basic;
        float ambientIntensity                    = D::AmbientIntensity;
        float diffuseIntensity                    = D::DiffuseIntensity;
        float normalIntensity                     = D::NormalIntensity;
        float textureTiling                       = D::TextureTiling;
        std::array<float, 3> surfaceColor         = D::SurfaceColor;
        std::array<float, 3> shoreColor           = D::ShoreColor;
        std::array<float, 3> deepColor            = D::DeepColor;
        const char* normalMapTexturePath          = D::NormalMapTexturePath;

        // non-owning pointer to already-loaded image
        CGraphicImage* normalMapImage             = nullptr;

        float worldUVScale                        = ShaderDefaults::Water::Basic::WorldUVScale;
    } basic;

    struct Displacement
    {
        using D = ShaderDefaults::Water::Displacement;
        float heightIntensity                     = D::HeightIntensity;
        float waveTiling                          = D::WaveTiling;
        float waveAmplitudeFactor                 = D::WaveAmplitudeFactor;
        float waveSteepness                       = D::WaveSteepness;
        std::array<float, 4> waveAmplitude        = D::WaveAmplitude;
        std::array<float, 4> wavesIntensity       = D::WavesIntensity;
        std::array<float, 4> wavesNoise           = D::WavesNoise;
        const char* heightMapTexturePath          = D::HeightMapTexturePath;

        // non-owning pointer to already-loaded image
        CGraphicImage* heightMapImage             = nullptr;
    } displacement;

    struct Refraction
    {
        using D = ShaderDefaults::Water::Refraction;
        float waterClarity                        = D::WaterClarity;
        float waterTransparency                   = D::WaterTransparency;
        std::array<float, 4> horizontalExtinction = D::HorizontalExtinction;
        std::array<float, 4> refractionReflection = D::RefractionReflection;
        float refractionScale                     = D::RefractionScale;
    } refraction;

    struct Reflection
    {
        using D = ShaderDefaults::Water::Reflection;
        float shininess                           = D::Shininess;
        std::array<float, 4> specularIntensity    = D::SpecularIntensity;
        float distortion                          = D::Distortion;
        float radianceFactor                      = D::RadianceFactor;
    } reflection;

    struct Foam
    {
        using D = ShaderDefaults::Water::Foam;
        std::array<float, 4> foamTiling           = D::FoamTiling;
        std::array<float, 4> foamRanges           = D::FoamRanges;
        std::array<float, 4> foamNoise            = D::FoamNoise;
        float foamSpeed                           = D::FoamSpeed;
        float foamIntensity                       = D::FoamIntensity;
        float shoreFade                           = D::ShoreFade;
    } foam;
};

// Global accessor (simple, explicit, no hidden behavior)
inline WaterShaderSettings& GetWaterShaderSettings()
{
    static WaterShaderSettings s_settings;
    return s_settings;
}

struct WaterPerFrameCB
{
    std::array<float, 4> slot0;          /* ==== VS: c0       | PS: -- ==== */
    std::array<float, 4> slot1;          /* ==== VS: c1       | PS: c0 ==== */
};

struct WaterDisplacementCB
{
    std::array<float, 4> slot0;          /* ==== VS: c2       | PS: -- ==== */
    std::array<float, 4> slot1;          /* ==== VS: c3       | PS: -- ==== */
    std::array<float, 4> slot2;          /* ==== VS: c4       | PS: -- ==== */
    std::array<float, 4> slot3;          /* ==== VS: c5       | PS: -- ==== */
    std::array<float, 4> slot4;          /* ==== VS: c6       | PS: -- ==== */
};

struct WaterMatricesCB
{
    std::array<float, 16> worldViewProj; /* ==== VS: c7..c10  | PS: -- ==== */
    std::array<float, 16> view;          /* ==== VS: c11..c14 | PS: -- ==== */
    std::array<float, 16> texTransform;  /* ==== VS: c15..c18 | PS: -- ==== */
};

struct WaterMaterialCB
{
    std::array<float, 4> slot0;          /* ==== VS: --       | PS: c1 ==== */
    std::array<float, 4> slot1;          /* ==== VS: --       | PS: c2 ==== */
    std::array<float, 4> slot2;          /* ==== VS: --       | PS: c3 ==== */
    std::array<float, 4> slot3;          /* ==== VS: --       | PS: c4 ==== */
    std::array<float, 4> slot4;          /* ==== VS: --       | PS: c5 ==== */
    std::array<float, 4> slot5;          /* ==== VS: --       | PS: c6 ==== */
};

static_assert(sizeof(WaterPerFrameCB) == 32);
static_assert(sizeof(WaterDisplacementCB) == 80);
static_assert(sizeof(WaterMatricesCB) == 192);
static_assert(sizeof(WaterMaterialCB) == 96);

struct WaterShaderInputs
{
    struct VS
    {
        WaterMatricesCB matrices;
        WaterPerFrameCB perFrame;
        WaterDisplacementCB displacement;
    } vs;

    struct PS
    {
        WaterMaterialCB material;
        WaterPerFrameCB perFrame;
    } ps;

    const WaterShaderSettings* settings = nullptr;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct SkyboxVSCB
{
    std::array<float, 16> worldViewProj; /* ==== VS: c0..c3   | PS: -- ==== */
};

static_assert(sizeof(SkyboxVSCB) == 64);

struct SkyboxShaderInputs
{
    SkyboxVSCB vs;
};

struct CloudVSCB
{
    std::array<float, 16> worldViewProj; /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 4>  uvScaleSpeed;  /* ==== VS: c4       | PS: -- ==== */
    std::array<float, 4>  timeSeconds;   /* ==== VS: c5       | PS: -- ==== */
};

static_assert(sizeof(CloudVSCB) == 96);

struct CloudPSCB
{
    std::array<float, 4> cloudTint; // PS c0
};

static_assert(sizeof(CloudPSCB) == 16);

struct CloudShaderInputs
{
    CloudVSCB vs;
    CloudPSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct EnvironmentAuthoring
{
    using D = ShaderDefaults::Environment;
    std::array<float, 4> windDirection = D::WindDirection;
    float windStrength                 = D::WindStrength;
};

struct EnvironmentRuntime
{
    float timeSeconds = 0.0f;
    std::array<float, 4> windDirection = { 0.f, 0.f, 0.f, 0.f };
    float windStrength = 0.0f;

    std::array<float, 3> sunDir = { 0.f, 0.f, 0.f };
    std::array<float, 3> sunColor = { 1.f, 1.f, 1.f };
};

// NOTE:
// - authoring: editable, persistent intent
// - runtime: per-frame derived state, simulation-owned
struct EnvironmentShaderSettings
{
    EnvironmentAuthoring authoring;
    EnvironmentRuntime   runtime;
};

inline EnvironmentShaderSettings& GetEnvironmentShaderSettings()
{
    static EnvironmentShaderSettings s_settings;
    return s_settings;
}

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct FrameShaderInputs
{
    // Matrices (raw, not transposed)
    std::array<float, 16> view;

    // Camera (world)
    std::array<float, 3> cameraPos;

    // Light (world)
    std::array<float, 3> sunDir;
    std::array<float, 3> sunColor;

    // Environment runtime (optional but useful to centralize)
    float timeSeconds = 0.0f;
    std::array<float, 4> windDirection = { 0.f, 0.f, 0.f, 0.f };
    float windStrength = 0.0f;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct WeaponTraceShaderSettings
{
    using D = ShaderDefaults::WeaponTrace;
    bool useTexture = D::UseTexture;
};

inline WeaponTraceShaderSettings& GetWeaponTraceShaderSettings()
{
    static WeaponTraceShaderSettings s_settings;
    return s_settings;
}

struct WeaponTraceVSCB
{
    std::array<float, 16> worldViewProj; // VS c0..c3
};

struct WeaponTracePSCB
{
    std::array<float, 4> slot0; // PS c0 (x = useTexture)
};

static_assert(sizeof(WeaponTraceVSCB) == 64);
static_assert(sizeof(WeaponTracePSCB) == 16);

struct WeaponTraceShaderInputs
{
    WeaponTraceVSCB vs;
    WeaponTracePSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct LensFlareVSCB
{
    std::array<float, 16> worldViewProj; // VS c0..c3
};

struct LensFlarePSCB
{
    std::array<float, 4> brightnessColor; // rgb=color, a=maxBrightness
};

static_assert(sizeof(LensFlareVSCB) == 64);
static_assert(sizeof(LensFlarePSCB) == 16);

struct LensFlareShaderInputs
{
    LensFlareVSCB vs;
    LensFlarePSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct ScreenPrimitiveVSCB
{
    std::array<float, 16> worldViewProj; // VS c0..c3
};

struct ScreenPrimitivePSCB
{
    std::array<float, 4> mode; // x = mode, yzw unused (PS c0)
    std::array<float, 4>  colorFactor;
};

static_assert(sizeof(ScreenPrimitiveVSCB) == 64);
static_assert(sizeof(ScreenPrimitivePSCB) == 32);

struct ScreenPrimitiveShaderInputs
{
    ScreenPrimitiveVSCB vs;
    ScreenPrimitivePSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct MiniMapShaderSettings
{
    bool useTexture = true;
    bool useMask = true;
};

inline MiniMapShaderSettings& GetMiniMapShaderSettings()
{
    static MiniMapShaderSettings s_settings;
    return s_settings;
}

struct MiniMapVSCB
{
    std::array<float, 16> worldViewProj; // c0..c3
    std::array<float, 16> world;         // c4..c7
    std::array<float, 16> texTransform;  // c8..c11
};

static_assert(sizeof(MiniMapVSCB) == 192);

struct MiniMapPSCB
{
    std::array<float, 4> colorFactor;  // c0 (replaces TEXTUREFACTOR)
    std::array<float, 4> flags;        // c1 (x=useTexture, y=useMask)
};

static_assert(sizeof(MiniMapPSCB) == 32);

struct MiniMapShaderInputs
{
    MiniMapVSCB vs;
    MiniMapPSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct TextVSCB
{
    // c0
    std::array<float, 4> invScreenSize; // (1/width, 1/height, 0, 0)
};

static_assert(sizeof(TextVSCB) == 16);

struct TextShaderInputs
{
    float invScreenW;
    float invScreenH;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct ModelVSCB
{

};

struct ModelPSCB
{

};

struct ModelShaderInputs
{
    ModelVSCB vs;
    ModelPSCB ps;
};
