row_major float4x4 ViewProj : register(c0); // c0..c3
row_major float4x4 World : register(c4); // c4..c7
row_major float4x4 ColorTexMatrix : register(c8); // c8..c11
row_major float4x4 AlphaTexMatrix : register(c12); // c12..c15

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

    float3 normal : TEXCOORD2;
    float3 worldPos : TEXCOORD3;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float4 worldPos = mul(float4(input.pos, 1.0), World);

    o.worldPos = worldPos.xyz;
    o.normal = mul(float4(input.normal, 0.0f), World).xyz;

    o.position = mul(worldPos, ViewProj);

    float4 colorUV = mul(worldPos, ColorTexMatrix);
    float4 alphaUV = mul(worldPos, AlphaTexMatrix);

    o.uvColor = colorUV.xy;
    o.uvAlpha = alphaUV.xy;

    return o;
}
