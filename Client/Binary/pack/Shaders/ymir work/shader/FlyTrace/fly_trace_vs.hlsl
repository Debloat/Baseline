row_major float4x4 g_viewProj : register(c0);
row_major float4x4 g_world : register(c4);

struct VS_INPUT
{
    float3 pos : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float4 color : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    float4 worldPos = mul(float4(input.pos, 1.0f), g_world);
    o.position = mul(worldPos, g_viewProj);
    o.color = input.color;
    return o;
}
