row_major float4x4 g_mViewProj : register(c0); // c0..c3
row_major float4x4 g_mWorld : register(c4); // c4..c7

struct VS_INPUT
{
    float3 pos : POSITION;
    float4 diffuse : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 pos : POSITION;
    float4 diffuse : COLOR0;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    float4 worldPos = mul(float4(input.pos, 1.0f), g_mWorld);
    o.pos = mul(worldPos, g_mViewProj);
    o.diffuse = input.diffuse;
    o.uv = input.uv;
    return o;
}
