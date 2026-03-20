row_major float4x4 viewProj : register(c0); // c0..c3
row_major float4x4 world : register(c4); // c4..c7
row_major float4x4 viewInverse : register(c8); // c8..c11
float4 texScale : register(c12); // c12

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 worldPos = mul(float4(input.position, 1.0f), world);

    output.position = mul(worldPos, viewProj);

    float4 camPos = mul(worldPos, viewInverse);

    output.uv = camPos.xy * float2(texScale.x, -texScale.x);

    return output;
}
