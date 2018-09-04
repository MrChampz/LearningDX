//========================================================
// Opaque VS
//========================================================

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
};

cbuffer cbPerPass : register(b1)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewProj;
	float4x4 gInvView;
	float4x4 gInvProj;
	float4x4 gInvViewProj;
	float3   gEyePosW;
	float    cbPerPassPad1;
	float2   gRenderTargetSize;
	float2   gInvRenderTargetSize;
	float    gNearZ;
	float    gFarz;
	float    gTotalTime;
	float    gDeltaTime;
};

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);

	return vout;
}