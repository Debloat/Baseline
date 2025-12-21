float4 g_vBrightness : register(c0); // rgb=color, a=maxBrightness
sampler2D g_sDiffuse : register(s0);

struct PS_IN
{
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN i) : COLOR0
{
    float4 tex = tex2D(g_sDiffuse, i.uv);

    // Combine texture and vertex first (original behavior)
    float4 base = tex * i.color;

    // Apply environment brightness to emitted energy only
    base.rgb *= g_vBrightness.rgb * g_vBrightness.a;

    // Do NOT scale alpha with brightness
    // Alpha remains texture * vertex mask
    return base;
}

