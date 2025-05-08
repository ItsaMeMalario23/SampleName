struct asciidata
{
    float4 color;
    float2 position;
    float scale;
    uint charID;
};

struct output
{
    float2 texcoord : TEXCOORD0;
    float4 color : TEXCOORD1;
    float4 position : SV_Position;
};

StructuredBuffer<asciidata> data : register(t0, space0);

static const uint indices[6] = {0, 1, 2, 3, 2, 1};

static const float2 vertexpos[4] = {{0.0f, 2.0f}, {2.0f, 2.0f}, {0.0f, 0.0f}, {2.0f, 0.0f}};

output main(uint id : SV_VertexID)
{
    uint idx = id / 6;

    output retval;

    if (data[idx].charID == 0) {
        retval.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        return retval;
    }

    uint vert = indices[id % 6];

    float u = (data[idx].charID % 19) * (1.0f / 19.0f) - (1.0f / 304.0f);
    float v = (data[idx].charID / 19) * (1.0f / 5.0f);

    float2 texcoords[4] = {
        {u + 0.0f          , v + 0.0f         },
        {u + (1.0f / 19.0f), v + 0.0f         },
        {u + 0.0f          , v + (1.0f / 5.0f)},
        {u + (1.0f / 19.0f), v + (1.0f / 5.0f)},
    };

    float2 tmp = float2((vertexpos[vert].x * data[idx].scale) + data[idx].position.x - 1.0f, (vertexpos[vert].y * data[idx].scale * 1.7777777778f) + data[idx].position.y - 1.0f);

    //retval.position = float4((vertexpos[vert] * data[idx].scale) + data[idx].position - 1.0f, 0.0f, 1.0f);
    retval.position = float4(tmp, 0.0f, 1.0f);
    retval.texcoord = texcoords[vert];
    retval.color = data[idx].color;

    return retval;
}