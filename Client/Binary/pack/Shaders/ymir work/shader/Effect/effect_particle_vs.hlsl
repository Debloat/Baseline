row_major float4x4 g_viewProj;
row_major float4x4 g_world;

struct VS_INPUT
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    o.position = mul(float4(input.position, 1.0f), g_viewProj);
    o.texcoord = input.texcoord;
    return o;
}
