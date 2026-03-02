sampler2D g_texture : register(s0);
float4 g_textureFactor;

struct PS_INPUT
{
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR0
{
    float4 tex = tex2D(g_texture, input.texcoord);
    return tex * g_textureFactor;
}
