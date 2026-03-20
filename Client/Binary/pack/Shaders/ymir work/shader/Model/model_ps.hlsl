#include "../Common/light.hlsl"

sampler2D DiffuseTexture0 : register(s0);
sampler2D DiffuseTexture1 : register(s1);

float4 TextureFlags : register(c0);
float4 SpecularParams : register(c1); // x = enabled, y = power, z = intensity

struct PS_INPUT
{
    float2 uv : TEXCOORD0;

    float3 normal : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};

float4 main(PS_INPUT input) : COLOR
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    if (TextureFlags.x > 0.5f)
    {
        color *= tex2D(DiffuseTexture0, input.uv);
    }

    if (TextureFlags.y > 0.5f)
    {
        color *= tex2D(DiffuseTexture1, input.uv);
    }

    float3 N = normalize(input.normal);
    float3 V = normalize(GetCameraPos() - input.worldPos);
    float3 L = GetSunDir();

    float3 ambient = GetAmbientColor();

    float3 lit = color.rgb * ambient;

    if (SpecularParams.x > 0.5f)
    {
        LightResult l = ComputeDirectionalLight(
        N,
        L,
        V,
        GetSunColor(),
        SpecularParams.y,
        SpecularParams.z
    );

        lit += color.rgb * l.diffuse;
        lit += l.specular;
    }
    else
    {
        float3 lightDiffuse = ComputeDiffuse(N, L) * GetSunColor();
        lit += color.rgb * lightDiffuse;
    }

    return float4(lit, color.a);
}
