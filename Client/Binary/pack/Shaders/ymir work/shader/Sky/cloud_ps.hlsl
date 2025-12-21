float4 g_vCloudTint : register(c0); // rgb = tint multiplier, a unused

sampler2D sTex0 : register(s0);

struct PS_IN
{
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

float4 main(PS_IN IN) : COLOR0
{
    float4 texc = tex2D(sTex0, IN.UV);

    // Approximate MODULATEINVALPHA_ADDCOLOR style behavior:
    // color = tex.rgb + (diffuse.rgb * tint.rgb) * (1 - tex.a)
    float3 rgb = (texc.rgb * g_vCloudTint.rgb) + IN.Color.rgb * (1.0f - texc.a);

    return float4(rgb, texc.a);
}
