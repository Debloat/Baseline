row_major float4x4 g_viewProj : register(c0);
row_major float4x4 g_world : register(c4);

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    float4 worldPos = mul(float4(input.pos, 1.0f), g_world);
    o.position = mul(worldPos, g_viewProj);
    o.uv0 = input.uv0;
    o.uv1 = input.uv1;
    return o;
}
