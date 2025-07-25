cbuffer UniformBlock : register(b0, space1)
{
    float4x4 rendermatrix : packoffset(c0);
};

cbuffer UniformBlock : register(b1, space1)
{
    float4x4 modeltransform : packoffset(c0);
};

struct ret
{
    float4 pos : SV_Position;
    float4 color : TEXCOORD0;
};

#define LINE_LEN (2.0f)

ret main(uint id : SV_VertexID)
{
    ret r;

    float4 dir;

    if (id == 0 || id == 1) {
        dir = float4(LINE_LEN, 0.0f, 0.0f, 1.0f);
        r.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    } else if (id == 2 || id == 3) {
        dir = float4(0.0f, LINE_LEN, 0.0f, 1.0f);
        r.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    } else {
        dir = float4(0.0f, 0.0f, LINE_LEN, 1.0f);
        r.color = float4(0.0f, 0.0f, 1.0f, 1.0f);
    }

    float4 pos;

    if (id % 2 == 0) {
        pos = mul(modeltransform, float4(0.0f, 0.0f, 0.0f, 1.0f));
    } else {
        pos = mul(modeltransform, dir);
    }

    r.pos = mul(rendermatrix, pos);

    return r;
}