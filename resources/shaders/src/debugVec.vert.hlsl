cbuffer UniformBlock : register(b0, space1)
{
    float4x4 rendermatrix : packoffset(c0);
};

cbuffer UniformBlock : register(b1, space1)
{
    float4 vec[8] : packoffset(c0);
};

struct ret
{
    float4 pos : SV_Position;
    float4 color : TEXCOORD0;
};

ret main(uint id : SV_VertexID)
{
    ret r;

    uint v = id / 2;

    if (v == 0) {
        r.color = float4(1.0f, 0.0f, 1.0f, 1.0f);
    } else if (v == 1) {
        r.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    } else if (v == 2) {
        r.color = float4(0.0f, 0.0f, 1.0f, 1.0f);
    } else {
        r.color = float4(1.0f, 0.0f, .0f, 1.0f);
    }

    float4 pos;

    if (id % 2 == 0) {
        pos = float4(vec[v].xyz, 1.0f);
    } else {
        pos = float4(vec[v + 1].xyz, 1.0f);
    }

    r.pos = mul(rendermatrix, pos);

    return r;
}