sampler2D g_tex : register(s0);

float4 main(float2 uv : TEXCOORD0) : COLOR0
{
    return tex2D(g_tex, uv);
}
