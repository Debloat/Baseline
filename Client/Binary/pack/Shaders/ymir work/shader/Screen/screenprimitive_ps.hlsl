sampler2D s0 : register(s0);

float4 g_vMode : register(c0);
float4 g_vColorFactor : register(c1);

// x = 0 → color only
// x = 1 → texture * color
// x = 2 → texture add colorFactor
// y = debug constant color flag (1 = use constant)
// zw unused

float4 main(float4 color : COLOR0, float2 uv : TEXCOORD0) : COLOR0
{
    if (g_vMode.y == 1)
        return float4(1.0f, 1.0f, 0.0f, 1.0f); // debug yellow

    if (g_vMode.x == 0)
        return color;

    float4 tex = tex2D(s0, uv);

    if (g_vMode.x == 1)
        return tex * color * g_vColorFactor;

    if (g_vMode.x == 2)
        return (tex + color) * g_vColorFactor;

    color *= g_vColorFactor;
    return color;
}
