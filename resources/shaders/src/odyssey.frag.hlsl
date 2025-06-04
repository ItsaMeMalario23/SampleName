Texture2D<float4> texture : register(t0, space2);
SamplerState smp : register(s0, space2);

float4 main(float2 texcoord : TEXCOORD0) : SV_Target0
{
    return texture.Sample(smp, texcoord);
}