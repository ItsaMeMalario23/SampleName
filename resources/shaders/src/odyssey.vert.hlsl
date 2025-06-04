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
    uint corner : TEXCOORD1;
};

struct ret
{
    float4 pos : SV_Position;
    float2 texcoord: TEXCOORD0;
};

ret main(arg a)
{
    ret r;

    r.pos = mul(rendermatrix, mul(modeltransform, float4(a.pos, 1.0f)));

    if (a.corner == 0)
        r.texcoord = float2(0.0f, 1.0f);
    else if (a.corner == 1)
        r.texcoord = float2(1.0f, 1.0f);
    else if (a.corner == 2)
        r.texcoord = float2(1.0f, 0.0f);
    else
        r.texcoord = float2(0.0f, 0.0f);

    return r;
}