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

    r.texcoord = a.uv;

    return r;
}