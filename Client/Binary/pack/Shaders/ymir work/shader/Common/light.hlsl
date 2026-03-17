#ifndef LIGHT_HLSL
#define LIGHT_HLSL

float4 g_frame0 : register(c1); // xyz = cameraPos
float4 g_frame1 : register(c2); // xyz = sunDir
float4 g_frame2 : register(c3); // xyz = sunColor
float4 g_frame3 : register(c4); // xyz = ambientColor

float3 GetCameraPos()
{
    return g_frame0.xyz;
}
float3 GetSunDir()
{
    return g_frame1.xyz;
}
float3 GetSunColor()
{
    return g_frame2.xyz;
}
float3 GetAmbientColor()
{
    return g_frame3.xyz;
}

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

float3 SafeNormalize(float3 v)
{
    return normalize(v + 1e-6);
}

// ------------------------------------------------------------
// Diffuse (Lambert)
// ------------------------------------------------------------

float ComputeDiffuse(float3 normal, float3 lightDir)
{
    // lightDir is assumed to point FROM surface TO sun
    // (matches your CPU sunDir usage)
    return saturate(dot(normal, lightDir));
}

// ------------------------------------------------------------
// Specular (Blinn-Phong, stable)
// ------------------------------------------------------------

float ComputeSpecular(
    float3 normal,
    float3 lightDir,
    float3 viewDir,
    float specPower)
{
    float3 H = SafeNormalize(lightDir + viewDir);
    float NdotH = saturate(dot(normal, H));

    return pow(NdotH, specPower);
}

// ------------------------------------------------------------
// Combined lighting
// ------------------------------------------------------------

struct LightResult
{
    float3 diffuse;
    float3 specular;
};

// NOTE:
// - normal, viewDir must be normalized before calling
// - lightDir should be normalized (sunDir)
// - specPower ~ [8..128]
// - specIntensity ~ [0..1]

LightResult ComputeDirectionalLight(
    float3 normal,
    float3 lightDir,
    float3 viewDir,
    float3 lightColor,
    float specPower,
    float specIntensity)
{
    LightResult r;

    float NdotL = ComputeDiffuse(normal, lightDir);

    // Diffuse
    r.diffuse = lightColor * NdotL;

    // Specular
    float spec = ComputeSpecular(normal, lightDir, viewDir, specPower);
    r.specular = lightColor * (spec * specIntensity);

    return r;
}

// ------------------------------------------------------------
// Energy-aware simple lighting (optional helper)
// ------------------------------------------------------------

float3 ComputeLitColor(
    float3 baseColor,
    float3 normal,
    float3 lightDir,
    float3 viewDir,
    float3 lightColor,
    float specPower,
    float specIntensity)
{
    LightResult l = ComputeDirectionalLight(
        normal,
        lightDir,
        viewDir,
        lightColor,
        specPower,
        specIntensity);

    // Simple energy split (not full PBR, but stable)
    float3 diffuse = baseColor * l.diffuse;
    float3 specular = l.specular;

    float ambientStrength = 0.2f;
    float3 ambient = baseColor * ambientStrength;

    return ambient + diffuse + specular;
}

#endif // LIGHT_HLSL
