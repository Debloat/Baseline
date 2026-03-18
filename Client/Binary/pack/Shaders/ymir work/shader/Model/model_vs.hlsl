row_major float4x4 WorldViewProj : register(c0);

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;

    float3 normal : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    float4 worldPos = float4(input.pos, 1.0f);

    o.position = mul(worldPos, WorldViewProj);
    o.uv = input.uv;

    o.worldPos = worldPos.xyz;
    o.normal = input.normal;
    return o;
}
