#pragma once
#include <array>

namespace ShaderDefaults
{
    struct Environment
    {
        static constexpr float TimeSeconds                  = 0.0f;
        static constexpr std::array<float, 4> WindDirection = { -4.5f, -6.0f, 0.0f, 0.0f };
        static constexpr float WindStrength                 = 0.1f;
    };

    struct Water
    {
        struct Features
        {
            static constexpr bool Displacement = true;
            static constexpr bool MeanSky      = true;
            static constexpr bool Filtering    = true;
            static constexpr bool BlinnPhong   = true;
            static constexpr int DebugView     = 0;
        };

        struct Basic
        {
            static constexpr float AmbientIntensity = 0.15f;
            static constexpr float DiffuseIntensity = 0.1f;
            static constexpr float NormalIntensity = 0.52f;
            static constexpr float TextureTiling = 3.5f;

            static constexpr std::array<float, 3> SurfaceColor = { 0.0f, 0.5f, 1.0f };
            static constexpr std::array<float, 3> ShoreColor = { 0.0f, 1.0f, 0.0f };
            static constexpr std::array<float, 3> DeepColor = { 0.0f, 0.0f, 0.0f };

            static constexpr const char* NormalMapTexturePath = "d:/ymir Work/shader/Water/water_normal.png";

            static constexpr float WorldUVScale = 1.0f / 512.0f;
        };

        struct Displacement
        {
            static constexpr float HeightIntensity = 0.3f;
            static constexpr float HeightAmplitude = 200 /*CTerrainImpl::CELLSCALE*/;
            static constexpr float WaveTiling = 1.0f;
            static constexpr float WaveAmplitudeFactor = 0.25f;
            static constexpr float WaveSteepness = 0.72f;

            static constexpr std::array<float, 4> WaveAmplitude = { 0.13f, 0.3f, 0.1f, 0.05f };
            static constexpr std::array<float, 4> WavesIntensity = { 5.0f, 3.0f, 2.0f, 1.3f };
            static constexpr std::array<float, 4> WavesNoise = { 0.15f, 0.32f, 0.15f, 0.15f };

            static constexpr const char* HeightMapTexturePath = "d:/ymir Work/shader/Water/water_height.png";
        };

        struct Refraction
        {
            static constexpr float WaterClarity = 0.25f;
            static constexpr float WaterTransparency = 10.0f;
            static constexpr std::array<float, 4> HorizontalExtinction = { 3.0f, 11.6f, 16.7f, 1.0f };
            static constexpr std::array<float, 4> RefractionReflection = { 0.17f, 0.11f, 0.31f, 1.0f };
            static constexpr float RefractionScale = 0.0048f;
        };

        struct Reflection
        {
            static constexpr float Shininess = 0.25f;
            static constexpr std::array<float, 4> SpecularIntensity = { 30.0f, 768.0f, 0.8f, 1.0f };
            static constexpr float Distortion = 0.0345f;
            static constexpr float RadianceFactor = 0.2f;
        };

        struct Foam
        {
            static constexpr std::array<float, 4> FoamTiling = { 2.0f, 0.5f, 0.0f, 1.0f };
            static constexpr std::array<float, 4> FoamRanges = { 2.25f, 3.3f, 10.4f, 1.0f };
            static constexpr std::array<float, 4> FoamNoise = { 0.37f, 0.5f, -0.3f, 0.05f };
            static constexpr float FoamSpeed = 10.0f;
            static constexpr float FoamIntensity = 0.4f;
            static constexpr float ShoreFade = 0.2f;
        };
    };

    struct WeaponTrace
    {
        static constexpr bool UseTexture = false;
    };
}
