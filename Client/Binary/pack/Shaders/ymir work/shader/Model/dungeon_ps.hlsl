sampler2D Texture0 : register(s0);
sampler2D Texture1 : register(s1);

struct PS_INPUT
{
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

float4 main(PS_INPUT input) : COLOR0
{
    float4 c0 = tex2D(Texture0, input.uv0);
    float4 c1 = tex2D(Texture1, input.uv1);
    return c0 * c1;
}
