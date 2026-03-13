sampler2D TerrainTexture : register(s0);
sampler2D AlphaTexture : register(s1);

float4 LayerState : register(c0);

struct PS_INPUT
{
    float2 uvColor : TEXCOORD0;
    float2 uvAlpha : TEXCOORD1;
};

float4 main(PS_INPUT input) : COLOR
{
    float4 color = tex2D(TerrainTexture, input.uvColor);
    float alpha = 1.0f;

    if (LayerState.x > 0.5f)
    {
        alpha = tex2D(AlphaTexture, input.uvAlpha).a;
    }

    return float4(color.rgb, alpha);
}