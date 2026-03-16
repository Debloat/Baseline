row_major float4x4 g_mWorldViewProj : register(c0);

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
    o.pos = mul(float4(input.pos, 1.0f), g_mWorldViewProj);
    o.diffuse = input.diffuse;
    o.uv = input.uv;
    return o;
}
