sampler2D sTex0 : register(s0);

struct PS_IN
{
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

float4 main(PS_IN IN) : COLOR0
{
    return tex2D(sTex0, IN.UV);
}
