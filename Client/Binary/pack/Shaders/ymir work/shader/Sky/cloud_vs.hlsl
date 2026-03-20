// -----------------------------------
row_major float4x4 g_mViewProj : register(c0);
row_major float4x4 g_mWorld : register(c4);

float4 g_vCloudParams0 : register(c8);    // x=uvScale.x, y=uvScale.y, z=uvSpeed.x, w=uvSpeed.y
float4 g_vCloudParams1 : register(c9);    // x=environmentTime
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
    // - unpack --------------------------
    float2 uvScale = g_vCloudParams0.xy;
    float2 uvSpeed = g_vCloudParams0.zw;
    float envTime  = g_vCloudParams1.x;
    // -----------------------------------

    VS_OUT OUT;
    float4 worldPos = mul(float4(IN.Pos, 1), g_mWorld);
    OUT.Pos = mul(worldPos, g_mViewProj);
    OUT.Color = IN.Color;
    OUT.UV = IN.UV * uvScale + uvSpeed * envTime;
    return OUT;
}
