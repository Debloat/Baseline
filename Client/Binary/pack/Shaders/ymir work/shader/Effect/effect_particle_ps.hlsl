sampler2D g_texture : register(s0);

float4 g_textureFactor : register(c0);
float4 g_ops : register(c1); // x = colorOpId

struct PS_INPUT
{
    float2 texcoord : TEXCOORD0;
};

float4 ApplyColorOp(float4 arg1, float4 arg2, float opId)
{
    // opId mapping is defined in C++ (see ParticleSystemInstance.cpp)
    // 0 = Modulate
    // 1 = Add
    // 2 = Subtract
    // 3 = Modulate2X
    // 4 = Modulate4X
    // 5 = AddSmooth

    if (opId < 0.5f)
        return arg1 * arg2; // Modulate
    if (opId < 1.5f)
        return arg1 + arg2; // Add
    if (opId < 2.5f)
        return arg1 - arg2; // Subtract
    if (opId < 3.5f)
        return (arg1 * arg2) * 2.0f; // Modulate2X
    if (opId < 4.5f)
        return (arg1 * arg2) * 4.0f; // Modulate4X
    /* opId < 5.5f */
    return arg1 + arg2 - (arg1 * arg2); // AddSmooth
}

float4 main(PS_INPUT input) : COLOR0
{
    float4 tex = tex2D(g_texture, input.texcoord);

    // FFP used COLORARG1=TFACTOR, COLORARG2=TEXTURE (from CEffectInstance wrapper)
    float4 rgb = ApplyColorOp(g_textureFactor, tex, g_ops.x);

    // For alpha, original wrapper also MODULATE alpha by TFACTOR/TEXTURE.
    // Keep it consistent: use same op on alpha channel too.
    return rgb;
}
