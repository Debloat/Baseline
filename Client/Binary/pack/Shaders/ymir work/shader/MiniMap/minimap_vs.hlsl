float4x4 g_mWorldViewProj : register(c0);
float4x4 g_mWorld : register(c4);
float4x4 g_mTexTransform : register(c8);

struct VS_IN
{
    float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : POSITION0;
    float2 uvTile : TEXCOORD0;
    float2 uvMask : TEXCOORD1;
};

VS_OUT main(VS_IN input)
{
    VS_OUT o;

    float4 worldPos = mul(float4(input.pos, 1.0), g_mWorld);
    o.pos = mul(worldPos, g_mWorldViewProj);

    o.uvTile = input.uv;

    float4 mask = mul(float4(worldPos.xy, 0, 1), g_mTexTransform);
    o.uvMask = mask.xy;

    return o;
}
