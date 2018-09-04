//=======================================================================
// Combines two images.
//=======================================================================

static const float2 gTexCoords[6] =
{
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(1.0f, 1.0f)
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(uint vid : SV_VertexID)
{
    VertexOut vout;

    vout.TexCoord = gTexCoords[vid];

    // Map [0, 1]^2 to NDC space.
    vout.PosH = float4(2.0f * vout.TexCoord.x - 1.0f, 1.0f - 2.0f * vout.TexCoord.y, 0.0f, 1.0f);

    return vout;
}