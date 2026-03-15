struct PS_INPUT
{
    float4 color : COLOR0;
};

float4 main(PS_INPUT input) : COLOR0
{
    return input.color;
}
