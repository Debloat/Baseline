// ------------------------------------------------------
// Constant buffers / uniforms
// ------------------------------------------------------
// -----------------------------------
float4 g_vWaterPerFrame0  : register(c0);  // xy = windDir.(xy) / z = windStrength / w = environmentTime
float4 g_vWaterPerFrame1  : register(c1);  // xyz = cameraPos / w = unused
float4 g_vWaterDisp0      : register(c2);  // x=heightIntensity, y=worldUVScale, z=waveTiling, w=waveSteepness
float4 g_vWaterDisp1      : register(c3);  // waveAmplitude
float4 g_vWaterDisp2      : register(c4);  // wavesIntensity
float4 g_vWaterDisp3      : register(c5);  // wavesNoise
float4 g_vWaterDisp4      : register(c6);  // x=waveAmplitudeFactor, y=heightAmplitude, z/w padding
float4x4 g_mWorldViewProj : register(c7);  // Combined world * view * projection matrix
float4x4 g_mView          : register(c11); // View matrix (used for fog depth computation)
float4x4 g_mTexTransform  : register(c15); // Legacy texture transform (kept for compatibility, not used here)
// -----------------------------------

// Heightmap texture (R channel used as displacement source)
sampler2D sWaterHeight : register(s2);

// ------------------------------------------------------
// Vertex input
// ------------------------------------------------------
struct VS_IN
{
    float3 Pos     : POSITION0; // world-space position
    float2 uv      : TEXCOORD0; // shoreline / alpha UV (CPU-generated)
    float4 Color   : COLOR0;    // vertex color (alpha = shoreline fade)
};

// ------------------------------------------------------
// Vertex output
// ------------------------------------------------------
struct VS_OUT
{
    float4 Pos : POSITION0; // clip-space position
    float2 Tex : TEXCOORD0; // world-space UV passed to pixel shader
    float4 Color : COLOR0; // forwarded vertex color
    float Height : TEXCOORD1; // debug: raw blended height before intensity
    float3 WorldPos : TEXCOORD2; // world-space position (displaced)
};

float SineWave(float2 pos, float2 dir, float A, float wavelength, float t)
{
    float k = 6.2831853f / max(wavelength, 0.0001f);
    float phase = k * dot(pos, dir) + t;
    return sin(phase) * A;
}

// Simple "Gerstner-like" horizontal push + vertical lift
float3 GerstnerWave(float2 pos, float2 dir, float A, float wavelength, float Q, float t)
{
    float k = 6.2831853f / max(wavelength, 0.0001f);
    float phase = k * dot(pos, dir) + t;

    float s = sin(phase);
    float c = cos(phase);

    float2 dispXZ = dir * (Q * A * c);
    float dispY = A * s;

    return float3(dispXZ.x, dispXZ.y, dispY);
}

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;

    // --------------------------------------------------
    // World-space UV generation (heightmap domain)
    //
    // - Derived directly from world position
    // - Stable across patches and terrain seams
    // - Independent from mesh resolution
    // --------------------------------------------------

    // - unpack --------------------------
    float2 windDir            = g_vWaterPerFrame0.xy;
    float windStrength        = g_vWaterPerFrame0.z;
    float envTime             = g_vWaterPerFrame0.w;
    float3 cameraPos          = g_vWaterPerFrame1.xyz;
    float heightIntensity     = g_vWaterDisp0.x;
    float waterWorldUVScale   = g_vWaterDisp0.y;
    float waveTiling          = g_vWaterDisp0.z;
    float waveSteepness       = g_vWaterDisp0.w;
    float4 waveAmplitude      = g_vWaterDisp1;
    float4 wavesIntensity     = g_vWaterDisp2;
    float4 wavesNoise         = g_vWaterDisp3;
    float waveAmplitudeFactor = g_vWaterDisp4.x;
    float heightAmplitude     = g_vWaterDisp4.y;
    // -----------------------------------

    // Canonical wind vector (world space)
    float2 wind = windDir * windStrength;

    // World-space UV (XY plane)
    float2 worldUV = vin.Pos.xy * waterWorldUVScale + wind * envTime;

    // --------------------------------------------------
    // Heightmap sampling
    //
    // - tex2Dlod is REQUIRED in vertex shaders
    // - LOD = 0 ensures highest detail
    // --------------------------------------------------

    // --------------------------------------------------
    // Multi-frequency heightmap sampling (anti-tiling)
    // --------------------------------------------------

    // Base world UV
    float2 uv0 = worldUV * 0.5f; // large waves
    float2 uv1 = worldUV * 1.3f; // mid waves
    float2 uv2 = worldUV * 3.7f; // small ripples

    float h0 = tex2Dlod(sWaterHeight, float4(uv0, 0.0f, 0.0f)).r;
    float h1 = tex2Dlod(sWaterHeight, float4(uv1, 0.0f, 0.0f)).r;
    float h2 = tex2Dlod(sWaterHeight, float4(uv2, 0.0f, 0.0f)).r;

    // Convert [0..1] ? [-1..1]
    h0 = (h0 - 0.5f) * 2.0f;
    h1 = (h1 - 0.5f) * 2.0f;
    h2 = (h2 - 0.5f) * 2.0f;

    // Convert from [0..1] ? [-1..1]
    // Assumes 0.5 is the neutral water plane
    // Weighted blend
    float height =
      h0 * 0.6f // dominant large motion
    + h1 * 0.3f // variation
    + h2 * 0.1f; // detail

    // --------------------------------------------------
    // Vertex displacement
    //
    // - Only Z is displaced
    // - Shoreline blending is preserved because
    //   alpha comes from vertex color, not height
    // --------------------------------------------------

    float3 displacedPos = vin.Pos;

    // Wind canonical
    float windLen = length(windDir);
    windDir = (windLen > 1e-6f) ? (windDir / windLen) : float2(1.0f, 0.0f);

    float windSpeed = max(windStrength, 0.0f);
    float t = envTime * windSpeed;

    // Water plane is XY (your current convention), height is Z
    float2 p = vin.Pos.xy * waveTiling;

    // Build 4 bands from your existing settings
    float4 amp = waveAmplitude * waveAmplitudeFactor;
    float4 inten = wavesIntensity;
    float4 nse = wavesNoise;

    // Wavelengths (stable base + user noise)
    float4 wl = float4(120.0f, 45.0f, 18.0f, 7.0f);
    wl += nse * float4(30.0f, 10.0f, 4.0f, 2.0f);

    // Steepness -> Q range (clamped for stability)
    float Qbase = saturate(waveSteepness) * 0.75f;

    // Two large gerstner-ish bands (0,1)
    float3 g0 = GerstnerWave(p, windDir, amp.x * inten.x, wl.x, Qbase, t * 0.6f);
    float3 g1 = GerstnerWave(p, windDir, amp.y * inten.y, wl.y, Qbase, t * 0.9f);

    // Two smaller sine bands (2,3)
    float s2 = SineWave(p, windDir, amp.z * inten.z, wl.z, t * 1.3f);
    float s3 = SineWave(p, windDir, amp.w * inten.w, wl.w, t * 1.8f);

    // Apply procedural displacement:
    // XY push from gerstner, Z lift from both
    displacedPos.x += (g0.x + g1.x);
    displacedPos.y += (g0.y + g1.y);
    displacedPos.z += (g0.z + g1.z) + s2 + s3;

    // Heightmap detail attenuation with distance (micro only)
    float dist = length(cameraPos.xy - displacedPos.xy);
    float attenDetail = saturate(1.0f - (dist - 200.0f) / 800.0f);
    attenDetail = lerp(0.15f, 1.0f, attenDetail);

    // Heightmap correctly scaled 0 to 1
    displacedPos.z += height
                * heightIntensity // 0..1 artistic control
                * heightAmplitude // world units
                * attenDetail;

    float4 worldPos = float4(displacedPos, 1.0f);

    // --------------------------------------------------
    // Standard transform pipeline
    // --------------------------------------------------

    // Clip-space transform
    vout.Pos = mul(worldPos, g_mWorldViewProj);

    // Pass world-space UV to pixel shader
    vout.Tex = worldUV;

    // Debug: expose raw blended height (total displacement in world units)
    vout.Height = displacedPos.z - vin.Pos.z;

    // World-space position after displacement
    vout.WorldPos = displacedPos;

    // Forward vertex color (alpha = shoreline fade)
    vout.Color = vin.Color;

    return vout;
}
