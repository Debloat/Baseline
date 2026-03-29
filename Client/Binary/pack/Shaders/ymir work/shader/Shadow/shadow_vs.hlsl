row_major float4x4 g_world : register(c0);
row_major float4x4 g_shadowViewProj : register(c4);

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float depth : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float4 worldPos = mul(float4(input.position, 1.0f), g_world);
    float4 shadowPos = mul(worldPos, g_shadowViewProj);

    o.position = shadowPos;
    o.depth = shadowPos.z / shadowPos.w;

    return o;
}
