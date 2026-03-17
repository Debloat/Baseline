#include "../Common/light.hlsl"

sampler2D TerrainTexture : register(s0);
sampler2D AlphaTexture : register(s1);

float4 LayerState : register(c0);

struct PS_INPUT
{
    float2 uvColor : TEXCOORD0;
    float2 uvAlpha : TEXCOORD1;

    float3 normal : TEXCOORD2;
    float3 worldPos : TEXCOORD3;
};

float4 main(PS_INPUT input) : COLOR
{
    float4 color = tex2D(TerrainTexture, input.uvColor);
    float alpha = 1.0f;

    if (LayerState.x > 0.5f)
    {
        alpha = tex2D(AlphaTexture, input.uvAlpha).a;
    }

    float3 N = normalize(input.normal);
    float3 V = normalize(GetCameraPos() - input.worldPos);
    float3 L = GetSunDir();

    float3 lightDiffuse = ComputeDiffuse(N, L) * GetSunColor();
    float3 ambient = GetAmbientColor();

    float3 lit = color.rgb * (ambient + lightDiffuse);

    return float4(lit, alpha);
}
