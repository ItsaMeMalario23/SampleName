struct asciidata
{
    float2 position;
    float4 color;
};

struct output
{
    float2 texcoord : TEXCOORD0;
    float4 color : TEXCOORD1;
    float4 position : SV_Position;
};

StructuredBuffer<asciidata> databuf : register(t0, space0);

static const uint indices[6] = {0, 1, 2, 3, 2, 1};

static const float2 vertexpos[4] = {{0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}};

static const float4 colors[4] = {{1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}};

output main(uint id : SV_VertexID)
{
    uint idx = id / 6;
    uint vert = indices[id % 6];

    asciidata dat = databuf[idx];

    float2 texcoords[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 1.0f},
    };

    output op;

    op.position = float4((vertexpos[vert].x / 4) + (dat.position.x) - 1.0f, (vertexpos[vert].y / 4) + (dat.position.y) - 1.0f, 0.0f, 1.0f);
    op.texcoord = texcoords[vert];
    //op.color = colors[idx % 4];
    op.color = dat.color;

    return op;
}