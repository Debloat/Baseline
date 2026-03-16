row_major float4x4 WorldViewProj : register(c0);
row_major float4x4 ColorTexMatrix : register(c4);
row_major float4x4 AlphaTexMatrix : register(c8);

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float2 uvColor : TEXCOORD0;
    float2 uvAlpha : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float4 worldPos = float4(input.pos, 1.0);

    o.position = mul(worldPos, WorldViewProj);

    float4 colorUV = mul(worldPos, ColorTexMatrix);
    float4 alphaUV = mul(worldPos, AlphaTexMatrix);

    o.uvColor = colorUV.xy;
    o.uvAlpha = alphaUV.xy;

    return o;
}
