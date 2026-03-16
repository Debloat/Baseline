row_major float4x4 WorldViewProj : register(c0);

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
    o.position = mul(float4(input.pos, 1.0f), WorldViewProj);
    o.color = input.color;
    return o;
}
