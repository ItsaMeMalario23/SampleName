cbuffer UniformBlock : register(b0, space1)
{
    float4x4 rendermatrix : packoffset(c0);
};

cbuffer UniformBlock : register(b1, space1)
{
    float4x4 modeltransform : packoffset(c0);
};

struct arg
{
    float3 pos : TEXCOORD0;
    uint color : TEXCOORD1;
};

struct ret
{
    float4 pos : SV_Position;
    float4 color: TEXCOORD0;
};

ret main(arg a)
{
    ret r;

    r.pos = mul(rendermatrix, mul(modeltransform, float4(a.pos, 1.0f)));
    r.color = float4(((float) ((a.color >> 24) & 0xFF)) / 255.0f, ((float) ((a.color >> 16) & 0xFF)) / 255.0f, ((float) ((a.color >> 8) & 0xFF)) / 255.0f, 1.0f);

    return r;
}