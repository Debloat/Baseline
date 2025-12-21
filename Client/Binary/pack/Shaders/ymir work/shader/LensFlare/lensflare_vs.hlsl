float4x4 g_mWorldViewProj : register(c0);

struct VS_IN
{
    float3 pos : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN i)
{
    VS_OUT o;
    o.pos = mul(float4(i.pos, 1.0f), g_mWorldViewProj);
    o.color = i.color;
    o.uv = i.uv;
    return o;
}
