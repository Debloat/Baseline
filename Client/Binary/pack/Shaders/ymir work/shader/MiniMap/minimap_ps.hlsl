sampler2D sTex0 : register(s0);
sampler2D sMask : register(s1);

float4 g_vColorFactor : register(c0);
float4 g_vFlags : register(c1);

struct PS_IN
{
    float2 uvTile : TEXCOORD0;
    float2 uvMask : TEXCOORD1;
};

float4 main(PS_IN input) : COLOR0
{
    float4 color = float4(1, 1, 1, 1);

    if (g_vFlags.x > 0.5)
        color = tex2D(sTex0, input.uvTile);

    if (g_vFlags.y > 0.5)
    {
        float mask = tex2D(sMask, input.uvMask).a;
        clip(mask - 0.01);
    }

    color *= g_vColorFactor;

    return color;
}
