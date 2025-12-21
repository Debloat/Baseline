sampler2D sDiffuse : register(s0);

struct PS_IN
{
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN input) : COLOR0
{
    float4 tex = tex2D(sDiffuse, input.uv);
    return tex * input.color;
}
