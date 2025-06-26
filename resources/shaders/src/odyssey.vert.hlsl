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
    float2 uv : TEXCOORD1;
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

/*
    if (a.corner == 0x191919ff)
        r.texcoord = float2(0.0f, 1.0f);
    else if (a.corner == 0x201919ff)
        r.texcoord = float2(1.0f, 1.0f);
    else if (a.corner == 0x211919ff)
        r.texcoord = float2(1.0f, 0.0f);
    else
        r.texcoord = float2(0.0f, 0.0f);*/

    r.texcoord = a.uv;

    return r;
}