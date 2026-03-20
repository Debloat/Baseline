row_major float4x4 g_mViewProj : register(c0);
row_major float4x4 g_mWorld : register(c4);

struct VS_IN
{
    float3 pos : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT o;
    float4 worldPos = mul(float4(input.pos, 1.0f), g_mWorld);
    o.pos = mul(worldPos, g_mViewProj);
    o.color = input.color;
    o.uv = input.uv;
    return o;
}
