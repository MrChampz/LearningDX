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
	float4 PosH	    : SV_POSITION;
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

// We expand each line into a quad (4 vertices), so the maximum number of vertices
// we output per geometry shader invocation is 4.
[maxvertexcount(4)]
void GS(line GeoIn gin[2], inout TriangleStream<GeoOut> stream)
{
	float height = 10.0f;
	float halfHeight = 0.5f * height;

	float3 up = float3(0.0f, 1.0f, 0.0f);

	// Generate the 4 quad vertices, using the base line vertices.
	float4 v[4];
	v[0] = float4(gin[0].PosL - halfHeight * up, 1.0f);
	v[1] = float4(gin[0].PosL + halfHeight * up, 1.0f);
	v[2] = float4(gin[1].PosL - halfHeight * up, 1.0f);
	v[3] = float4(gin[1].PosL + halfHeight * up, 1.0f);

	// Use the normal vectors of the line in all the vertices.
	float3 n[4];
	n[0] = gin[0].NormalL;
	n[1] = gin[0].NormalL;
	n[2] = gin[1].NormalL;
	n[3] = gin[1].NormalL;

	// Use the u-axis texture coordinates of the line in all the vertices.
	// Define the v-axis coordinate based on the vertex position.
	float2 texCoord[4];
	texCoord[0] = float2(gin[0].TexCoord.x, 1.0f);
	texCoord[1] = float2(gin[0].TexCoord.x, 0.0f);
	texCoord[2] = float2(gin[1].TexCoord.x, 1.0f);
	texCoord[3] = float2(gin[1].TexCoord.x, 0.0f);

	// Transform the quad texture coordinates by the TexTransform matrix.
	texCoord[0] = mul(float4(texCoord[0], 0.0f, 1.0f), gTexTransform).xy;
	texCoord[1] = mul(float4(texCoord[1], 0.0f, 1.0f), gTexTransform).xy;
	texCoord[2] = mul(float4(texCoord[2], 0.0f, 1.0f), gTexTransform).xy;
	texCoord[3] = mul(float4(texCoord[3], 0.0f, 1.0f), gTexTransform).xy;

	GeoOut gout;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		float4 posW = mul(v[i], gWorld);
		float3 normalW = mul(n[i], (float3x3)gWorld);

		gout.PosH = mul(posW, gViewProj);
		gout.PosW = posW.xyz;
		gout.NormalW = normalW;
		gout.TexCoord = texCoord[i];

		stream.Append(gout);
	}
}