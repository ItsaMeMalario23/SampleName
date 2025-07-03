Texture2D<float4> texture : register(t0, space2);
SamplerState smp : register(s0, space2);

struct input
{
    float2 texcoord : TEXCOORD0;
    float4 color : TEXCOORD1;
};

float4 main(input inp) : SV_Target0
{
    return inp.color * texture.Sample(smp, inp.texcoord);
}