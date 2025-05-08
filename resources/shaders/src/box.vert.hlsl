struct input
{
    float2 pos : TEXCOORD0;
};

struct output
{
    float4 pos : SV_Position;
};

output main(input inp)
{
    output retval;

    retval.pos = float4(inp.pos, 0.0f, 1.0f);

    return retval;
}