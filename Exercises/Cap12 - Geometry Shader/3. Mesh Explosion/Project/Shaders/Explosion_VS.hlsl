struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Just pass data over to geometry shader.
	vout.PosL = vin.PosL;
	vout.NormalL = vin.NormalL;
	vout.TexCoord = vin.TexCoord;

	return vout;
}