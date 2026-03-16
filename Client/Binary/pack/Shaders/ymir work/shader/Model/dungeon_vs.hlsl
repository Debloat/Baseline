row_major float4x4 WorldViewProj : register(c0);

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
    o.position = mul(float4(input.pos, 1.0f), WorldViewProj);
    o.uv0 = input.uv0;
    o.uv1 = input.uv1;
    return o;
}
