// -----------------------------------
row_major float4x4 g_mWorldViewProj : register(c0);
// -----------------------------------

struct VS_IN
{
    float3 Pos : POSITION0;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

struct VS_OUT
{
    float4 Pos : POSITION0;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

VS_OUT main(VS_IN IN)
{
    VS_OUT OUT;
    OUT.Pos = mul(float4(IN.Pos, 1.0f), g_mWorldViewProj);
    OUT.Color = IN.Color;
    OUT.UV = IN.UV;
    return OUT;
}
