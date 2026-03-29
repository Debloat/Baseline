float4 g_params0 : register(c0); // x = alphaCutoff, yzw unused

struct PS_INPUT
{
    float depth : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR0
{
    float d = saturate(input.depth);
    return float4(d, d, d, 1.0f);
}
