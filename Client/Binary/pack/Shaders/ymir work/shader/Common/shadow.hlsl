#ifndef SHADOW_HLSL
#define SHADOW_HLSL

//VS
row_major float4x4 g_shadowViewProj : register(c100);
row_major float4x4 g_shadowTex : register(c104);

//PS
float4 g_shadowParams0 : register(c104);
// x = depthBias
// y = shadowDarkness     (0 = fully black shadow, 1 = no darkening)
// z = invShadowMapSize
// w = unused

sampler2D sShadowMap : register(s7);

float DecodeShadowDepth(float4 c)
{
    return c.r;
}

float4 ComputeShadowClip(float3 worldPos)
{
    return mul(float4(worldPos, 1.0f), g_shadowViewProj);
}

float2 ComputeShadowUV(float3 worldPos)
{
    float4 st = mul(float4(worldPos, 1.0f), g_shadowTex);
    return st.xy / st.w;
}

float SampleShadowFactor(float3 worldPos)
{
    float4 shadowClip = ComputeShadowClip(worldPos);
    float2 shadowUV = ComputeShadowUV(worldPos);

    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f || shadowUV.y > 1.0f)
    {
        return 1.0f;
    }

    float receiverDepth = shadowClip.z / shadowClip.w;
    receiverDepth -= g_shadowParams0.x;

    float storedDepth = DecodeShadowDepth(tex2D(sShadowMap, shadowUV));

    return (receiverDepth <= storedDepth) ? 1.0f : g_shadowParams0.y;
}

#endif // SHADOW_HLSL
