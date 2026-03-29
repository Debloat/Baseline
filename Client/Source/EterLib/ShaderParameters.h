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
    std::array<float, 16> viewProj;      /* ==== VS: c7..c10  | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c11..c14 | PS: -- ==== */
    std::array<float, 16> view;          /* ==== VS: c15..c18 | PS: -- ==== */
    std::array<float, 16> texTransform;  /* ==== VS: c19..c22 | PS: -- ==== */
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
static_assert(sizeof(WaterMatricesCB) == 256);
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
    std::array<float, 16> viewProj; /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;    /* ==== VS: c4..c7   | PS: -- ==== */
};

static_assert(sizeof(SkyboxVSCB) == 128);

struct SkyboxShaderInputs
{
    SkyboxVSCB vs;
};

struct CloudVSCB
{
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
    std::array<float, 4>  uvScaleSpeed;  /* ==== VS: c8       | PS: -- ==== */
    std::array<float, 4>  timeSeconds;   /* ==== VS: c9       | PS: -- ==== */
};

static_assert(sizeof(CloudVSCB) == 160);

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

    std::array<float, 3> sunDir = { 0.0f, 0.0f, 1.0f };
    std::array<float, 3> sunColor = { 1.f, 1.f, 1.f };
    std::array<float, 3> ambientColor = { 0.2f, 0.2f, 0.25f };

    // --- SHADOW ---
    float shadowBias = 0.0015f;
    float shadowDarkness = 0.3f;
    float shadowMapSize = 4096.0f;
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

    // Shadow (raw, not transposed)
    std::array<float, 16> shadowViewProj;
    std::array<float, 16> shadowTex;

    // Camera (world)
    std::array<float, 3> cameraPos;

    // Light (world)
    std::array<float, 3> sunDir;
    std::array<float, 3> sunColor;
    std::array<float, 3> ambientColor;

    // Shadow (frame-global)
    float shadowBias = 0.0f;
    float shadowDarkness = 1.0f;
    float invShadowMapSize = 0.0f;

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
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
};

struct WeaponTracePSCB
{
    std::array<float, 4> slot0; // PS c0 (x = useTexture)
};

static_assert(sizeof(WeaponTraceVSCB) == 128);
static_assert(sizeof(WeaponTracePSCB) == 16);

struct WeaponTraceShaderInputs
{
    WeaponTraceVSCB vs;
    WeaponTracePSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct ScreenPrimitiveVSCB
{
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
};

struct ScreenPrimitivePSCB
{
    std::array<float, 4> mode; // x = mode, yzw unused (PS c0)
    std::array<float, 4>  colorFactor;
};

static_assert(sizeof(ScreenPrimitiveVSCB) == 128);
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
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
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

struct EffectParticleVSCB
{
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
};
static_assert(sizeof(EffectParticleVSCB) == 128);

struct EffectParticlePSCB
{
    std::array<float, 4> textureFactor; // rgba
    std::array<float, 4> ops;           // x = colorOpId, yzw unused
};
static_assert(sizeof(EffectParticlePSCB) == 32);

struct EffectParticleShaderInputs
{
    EffectParticleVSCB vs;
    EffectParticlePSCB ps;
};

struct EffectMeshVSCB
{
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
};
static_assert(sizeof(EffectMeshVSCB) == 128);

struct EffectMeshPSCB
{
    std::array<float, 4> textureFactor;
};
static_assert(sizeof(EffectMeshPSCB) == 16);

struct EffectMeshShaderInputs
{
    EffectMeshVSCB vs;
    EffectMeshPSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct ModelVSCB
{
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
};

struct ModelPSCB
{
    std::array<float, 4> textureFlags;
    std::array<float, 4> specularParams;  // x = enabled, y = power, z = intensity, w unused
};

struct ModelShaderInputs
{
    ModelVSCB vs;
    ModelPSCB ps;
};

struct DungeonVSCB
{
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
};

struct DungeonPSCB
{
};

struct DungeonShaderInputs
{
    DungeonVSCB vs;
    DungeonPSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct SnowParticleVSCB
{
    std::array<float, 16> viewProj;      /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;         /* ==== VS: c4..c7   | PS: -- ==== */
};
static_assert(sizeof(SnowParticleVSCB) == 128);

struct SnowParticleShaderInputs
{
    SnowParticleVSCB vs;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct TerrainVSCB
{
    std::array<float, 16> viewProj;       /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;          /* ==== VS: c4..c7   | PS: -- ==== */
    std::array<float, 16> colorTexMatrix; /* ==== VS: c8..c11  | PS: -- ==== */
    std::array<float, 16> alphaTexMatrix; /* ==== VS: c12..c15 | PS: -- ==== */
};

struct TerrainPSCB
{
    std::array<float, 4> layerState;
};

struct TerrainShaderInputs
{
    TerrainVSCB vs;
    TerrainPSCB ps;
};

struct TerrainMarkedAreaVSCB
{
    std::array<float, 16> viewProj;       /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;          /* ==== VS: c4..c7   | PS: -- ==== */
    std::array<float, 16> viewInverse;
    std::array<float, 4> texScale; // x= texscale yzw = padding
};

struct TerrainMarkedAreaPSCB
{
    std::array<float, 4> alpha; // x= alpha yzw = padding
};

struct TerrainMarkedAreaShaderInputs
{
    TerrainMarkedAreaVSCB vs;
    TerrainMarkedAreaPSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct FlyTraceVSCB
{
    std::array<float, 16> viewProj;       /* ==== VS: c0..c3   | PS: -- ==== */
    std::array<float, 16> world;          /* ==== VS: c4..c7   | PS: -- ==== */
};

struct FlyTracePSCB
{
};

struct FlyTraceShaderInputs
{
    FlyTraceVSCB vs;
    FlyTracePSCB ps;
};

/* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

struct ShadowVSCB
{
    std::array<float, 16> world;          // c0..c3
    std::array<float, 16> shadowViewProj; // c4..c7
};

struct ShadowPSCB
{
    std::array<float, 4> params0; // c0 : x = alphaCutoff, yzw unused for now
};

struct ShadowShaderInputs
{
    ShadowVSCB vs;
    ShadowPSCB ps;
};
