struct asciidata
{
    float2 position;
};

struct Input
{
    float3 Position : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
    uint Index : SV_InstanceID;
};

struct Output
{
    float2 TexCoord : TEXCOORD0;
    float4 Position : SV_Position;
};

StructuredBuffer<asciidata> databuf : register(t0, space0);

Output main(Input input)
{
    Output output;
    output.TexCoord = input.TexCoord;

    //float3 pos = (input.Position * 0.25f) - float3(0.75f, 0.75f, 0.0f);

    //pos.x += (float(input.Index % 4) * 0.5f);
    //pos.y += (floor(float(input.Index / 4)) * 0.5f);

    //output.Position = float4(pos, 1.0f);

    asciidata dat = databuf[input.Index];

    output.Position = float4(dat.position, 0.0f, 1.0f);

    return output;
}