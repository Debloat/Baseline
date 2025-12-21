float g_useTexture : register(c0);

sampler2D g_diffuseSampler : register(s0);

struct PS_INPUT
{
    float4 diffuse : COLOR0;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR0
{
    if (g_useTexture > 0.5f)
        return tex2D(g_diffuseSampler, input.uv);

    return input.diffuse;
}
