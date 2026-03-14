sampler2D DiffuseTexture0 : register(s0);
sampler2D DiffuseTexture1 : register(s1);

float4 TextureFlags : register(c0);

struct PS_INPUT
{
    float2 uv : TEXCOORD0;
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

    return color;
}
