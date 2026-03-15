sampler2D markedTex : register(s0);

float4 alpha : register(c0); // c0

struct PS_INPUT
{
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR
{
    float4 tex = tex2D(markedTex, input.uv);
    tex.a *= alpha.x;
    return tex;
}
