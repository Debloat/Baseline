float4 g_vInvScreenSize : register(c0); // x=1/width, y=1/height

struct VS_IN
{
    float3 pos : POSITION0; // pixel space
    float4 color : COLOR0; // D3DCOLOR -> normalized
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT o;

    const float invW = g_vInvScreenSize.x;
    const float invH = g_vInvScreenSize.y;

    // D3D clip space: x [-1..1], y [1..-1] for top->bottom pixels
    float clipX = input.pos.x * 2.0f * invW - 1.0f;
    float clipY = 1.0f - input.pos.y * 2.0f * invH;

    o.pos = float4(clipX, clipY, 0.0f, 1.0f);
    o.color = input.color;
    o.uv = input.uv;
    return o;
}
