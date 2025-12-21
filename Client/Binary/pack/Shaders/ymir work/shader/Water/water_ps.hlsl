// -----------------------------------
float4 g_vWaterPerFrame1 : register(c0); // xyz = cameraPos / w = unused

float4 g_vWaterMaterial0 : register(c1); // specularParams (x=intensity, y=exponent, z/w unused or future)
float4 g_vWaterMaterial1 : register(c2); // lightDir.xyz w=unused
float4 g_vWaterMaterial2 : register(c3); // lightColor.rgb w=unused
float4 g_vWaterMaterial3 : register(c4); // x=normalIntensity, y=useBlinnPhong, z=debugView, w=waterClarity
float4 g_vWaterMaterial4 : register(c5); // shallowColor.rgb w=unused
float4 g_vWaterMaterial5 : register(c6); // deepColor.rgb w=unused
// -----------------------------------

sampler2D sSky : register(s0);
sampler2D sWaterNormal : register(s1);

float FresnelSchlick(float cosTheta, float F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float2 SkyCrossUV(float3 dir)
{
    dir = normalize(dir);

    // Convert from Z-up (engine space) to Y-up (atlas convention)
    dir = float3(dir.x, -dir.z, dir.y);

    float ax = abs(dir.x);
    float ay = abs(dir.y);
    float az = abs(dir.z);

    // Face-local UV in [-1..1]
    float2 uv;
    float col = 0.0f;
    float row = 0.0f;

    // Pick dominant axis
    if (az >= ax && az >= ay)
    {
        // Z faces
        if (dir.z >= 0.0f)
        {
            // +Z (front) -> col 1, row 1
            col = 1.0f;
            row = 1.0f;
            uv = float2(dir.x / az, -dir.y / az);
        }
        else
        {
            // -Z (back) -> col 3, row 1
            col = 3.0f;
            row = 1.0f;
            uv = float2(-dir.x / az, -dir.y / az);
        }
    }
    else if (ax >= ay)
    {
        // X faces
        if (dir.x >= 0.0f)
        {
            // +X (right) -> col 2, row 1
            col = 2.0f;
            row = 1.0f;
            uv = float2(-dir.z / ax, -dir.y / ax);
        }
        else
        {
            // -X (left) -> col 0, row 1
            col = 0.0f;
            row = 1.0f;
            uv = float2(dir.z / ax, -dir.y / ax);
        }
    }
    else
    {
        // Y faces
        if (dir.y >= 0.0f)
        {
            // +Y (top) -> col 1, row 0
            col = 1.0f;
            row = 0.0f;
            uv = float2(dir.x / ay, dir.z / ay);
        }
        else
        {
            // -Y (bottom) -> col 1, row 2
            col = 1.0f;
            row = 2.0f;
            uv = float2(dir.x / ay, -dir.z / ay);
        }
    }

    // Map face-local [-1..1] -> [0..1]
    uv = uv * 0.5f + 0.5f;

    // Map into 4x3 atlas
    const float tileW = 1.0f / 4.0f;
    const float tileH = 1.0f / 3.0f;

    float2 atlasUV;
    atlasUV.x = (col + uv.x) * tileW;
    atlasUV.y = (row + uv.y) * tileH;

    return atlasUV;
}

float4 main(
    float2 Tex : TEXCOORD0,
    float4 Color : COLOR0,
    float Height : TEXCOORD1,
    float3 WorldPos : TEXCOORD2) : COLOR0
{
    // - unpack --------------------------
    float3 cameraPos      = g_vWaterPerFrame1.xyz;
    float4 specParams     = g_vWaterMaterial0;
    float3 lightDir       = g_vWaterMaterial1.xyz;
    float3 lightColor     = g_vWaterMaterial2.xyz;
    float normalIntensity = g_vWaterMaterial3.x;
    float useBlinnPhong   = g_vWaterMaterial3.y;
    float debugView       = g_vWaterMaterial3.z;
    // -----------------------------------

    float alpha = Color.a;

    // --------------------------------------------------
    // Layered normal synthesis (Wind is baked in VS already)
    // --------------------------------------------------

    // Large waves
    float2 uv0 = Tex * 0.25f;
    float3 n0 = tex2D(sWaterNormal, uv0).xyz * 2.0f - 1.0f;

    // Mid waves
    float2 uv1 = Tex * 1.0f;
    float3 n1 = tex2D(sWaterNormal, uv1).xyz * 2.0f - 1.0f;

    // Micro ripples
    float2 uv2 = Tex * 4.0f;
    float3 n2 = tex2D(sWaterNormal, uv2).xyz * 2.0f - 1.0f;

    // Weighted blend (XY only)
    float2 slopeLarge = n0.xy * 0.6f;
    float2 slopeMid = n1.xy * 0.3f;
    float2 slopeMicro = n2.xy * 0.1f;

    // Distance attenuation only affects micro detail
    float dist = length(cameraPos - WorldPos);
    float attenMicro = saturate(1.0f - (dist - 200.0f) / 600.0f);
    // Near: %100 Micro slope Far: %20 Micro slope
    attenMicro = lerp(0.2f, 1.0f, attenMicro);

    slopeMicro *= attenMicro;

    // Final slope
    float2 slope = (slopeLarge + slopeMid + slopeMicro) * normalIntensity;

    // Final water normal (Z-up plane)
    float3 n = normalize(float3(slope.x, slope.y, 1.0f));

    // -----------------------------
    // Lighting (Phase 2)
    // -----------------------------

    float3 viewDir = normalize(cameraPos - WorldPos);
    float3 nlightDir = normalize(-lightDir);

    // Fresnel (water ~ 0.02)
    float cosTheta = saturate(dot(n, viewDir));
    float fresnel = FresnelSchlick(cosTheta, 0.02f);

    // Blinn-Phong specular
    float3 halfVec = normalize(nlightDir + viewDir);

    // UI provides:
    //   x = specular intensity (linear, >= 0)
    //   y = specular exponent (Blinn/Phong, >= 1)
    float specIntensity = specParams.x;

    // Map perceptual [0..1] -> Blinn exponent [8..128]
    float specExponent = max(specParams.y, 1.0f);

    float ndh = saturate(dot(n, halfVec));

    float spec;

    if (useBlinnPhong > 0.5f)
    {
    // Blinn-Phong uses N·H
        spec = pow(ndh, specExponent);
    }
    else
    {
    // Phong uses R·V
        float3 refl = reflect(-nlightDir, n);
        float ndv = saturate(dot(refl, viewDir));
        spec = pow(ndv, specExponent);
    }

    float3 specular = spec * specIntensity * lightColor;

    // ----------------------------------
    // Phase 6 - Depth-based absorption
    // ----------------------------------

    // Height from VS = world displacement delta
    float waveThickness = abs(Height);

    // Convert to 0..1 range (tuning constant only)
    float waterClarity = g_vWaterMaterial3.w;

    // Convert clarity (0..1) to absorption strength
    // 1 = very clear (low absorption)
    // 0 = very murky (high absorption)
    float absorptionStrength = (1.0f - waterClarity) * 0.05f;

    float transmittance = exp(-absorptionStrength * waveThickness);

    // Use authoring colors (from CPU settings if desired later)
    float3 shallowColor = g_vWaterMaterial4.xyz;
    float3 deepColor = g_vWaterMaterial5.xyz;

    // 1 = shallow, 0 = deep
    float3 absorptionTint = lerp(deepColor, shallowColor, transmittance);

    // Base water color tinted
    float3 base = absorptionTint;

    // Environment reflection from sky atlas
    float3 reflDir = reflect(viewDir, n);
    float2 skyUV = SkyCrossUV(reflDir);
    float3 skyRefl = tex2D(sSky, skyUV).rgb;

    // Energy conserving fresnel split
    float3 diffusePart = base * (1.0f - fresnel);

    // Keep your sun specular, but let fresnel boost reflection
    float3 reflectionPart = (skyRefl + specular) * fresnel;

    float3 color = diffusePart + reflectionPart;

    // -----------------------------
    // Debug outputs
    // -----------------------------
    if (debugView > 0.5f && debugView < 1.5f)
    {
        // Normal visualization in 0..1
        return float4(n * 0.5f + 0.5f, 1.0f);
    }
    if (debugView > 1.5f && debugView < 2.5f)
    {
        // Height grayscale (Height is in approx [-1..1] if your heightmap is centered at 0.5)
        float h = saturate(Height * 0.5f + 0.5f);
        return float4(h, h, h, 1.0f);
    }
    
    if (debugView > 2.5f && debugView < 3.5f)
    {
    // UV visualization (should show gradients / motion)
        float2 u = frac(Tex);
        return float4(u.x, u.y, 0.0f, 1.0f);
    }
    
    if (debugView > 3.5f && debugView < 4.5f)
    {
        float specDbg = pow(ndh, specExponent) * specIntensity;
        return float4(specDbg, specDbg, specDbg, 1.0f);
    }

    if (debugView > 4.5f && debugView < 5.5f)
    {
    // Fresnel visualization (grayscale)
        return float4(fresnel, fresnel, fresnel, 1.0f);
    }

    return float4(color, alpha);
}
