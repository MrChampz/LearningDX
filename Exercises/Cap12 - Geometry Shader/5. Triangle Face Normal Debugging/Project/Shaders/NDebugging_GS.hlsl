// Include structures and functions for lighting.
#include "LightingUtil.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
};

cbuffer cbPass : register(b2)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewProj;
	float4x4 gInvView;
	float4x4 gInvProj;
	float4x4 gInvViewProj;
	float3   gEyePosW;
	float    cbPerObjectPad1;
	float2   gRenderTargetSize;
	float2   gInvRenderTargetSize;
	float    gNearZ;
	float    gFarz;
	float    gTotalTime;
	float    gDeltaTime;

	float4 gAmbientLight;

	// Allow application to change fog parameters once per frame.
	// For example, we may only use fog for certain times of day.
	float4 gFogColor;
	float  gFogStart;
	float  gFogRange;
	float2 cbPerObjectPad2;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light gLights[MaxLights];
};

struct GeoIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct GeoOut
{
	float4 PosH : SV_POSITION;
};

[maxvertexcount(2)]
void GS(triangle GeoIn gin[3], inout LineStream<GeoOut> stream)
{
	// Compute triangle face normal.
	float3 a = gin[1].PosL - gin[0].PosL;
	float3 b = gin[2].PosL - gin[0].PosL;
	float3 n = normalize(cross(a, b));

	// Compute triangle face midpoint.
	float3 p = (gin[0].PosL + gin[1].PosL + gin[2].PosL) / 3;

	// Compute line vertices.
	float3 v[2];
	v[0] = p;
	v[1] = p + 2.0f * n;

	GeoOut gout;
	[unroll]
	for (int i = 0; i < 2; ++i)
	{
		// Transform to world space.
		float3 posW = mul(float4(v[i], 1.0f), gWorld).xyz;

		// Transform to homogeneous clip space.
		gout.PosH = mul(float4(posW, 1.0f), gViewProj);

		stream.Append(gout);
	}
}