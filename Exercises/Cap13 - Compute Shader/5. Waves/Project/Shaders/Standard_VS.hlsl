// Include structures and functions for lighting.
#include "LightingUtil.hlsli"

Texture2D gDisplacementMap : register(t1);

SamplerState gSamplerPointWrap        : register(s0);
SamplerState gSamplerPointClamp       : register(s1);
SamplerState gSamplerLinearWrap       : register(s2);
SamplerState gSamplerLinearClamp      : register(s3);
SamplerState gSamplerAnisotropicWrap  : register(s4);
SamplerState gSamplerAnisotropicClamp : register(s5);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
    float2   gDisplacementMapTexelSize;
    float    gGridSpatialStep;
    float    cbPerObjectPad1;
};

cbuffer cbMaterial : register(b1)
{
	float4	 gDiffuseAlbedo;
	float3	 gFresnelR0;
	float	 gRoughness;
	float4x4 gMatTransform;
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
	float    cbPerPassPad1;
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
    float2 cbPerPassPad2;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light gLights[MaxLights];
};

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
	float4 PosH	    : SV_POSITION;
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

#ifdef DISPLACEMENT_MAP
    // Sample the displacement map using non-transformed [0, 1]^2 tex-coords.
    vin.PosL.y += gDisplacementMap.SampleLevel(gSamplerLinearWrap, vin.TexCoord, 1.0f).r;

    //
    // Estimate normal using finite difference.
    //

    float du = gDisplacementMapTexelSize.x;
    float dv = gDisplacementMapTexelSize.y;

    float l = gDisplacementMap.SampleLevel(gSamplerPointClamp, vin.TexCoord - float2(du, 0.0f), 0.0f).r;
    float r = gDisplacementMap.SampleLevel(gSamplerPointClamp, vin.TexCoord + float2(du, 0.0f), 0.0f).r;
    float t = gDisplacementMap.SampleLevel(gSamplerPointClamp, vin.TexCoord - float2(0.0f, dv), 0.0f).r;
    float b = gDisplacementMap.SampleLevel(gSamplerPointClamp, vin.TexCoord + float2(0.0f, dv), 0.0f).r;

    vin.NormalL = normalize(float3(-r + l, 2.0f * gGridSpatialStep, b - t));
#endif

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW = posW.xyz;

	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texCoord = mul(float4(vin.TexCoord, 0.0f, 1.0f), gTexTransform);
	vout.TexCoord = mul(texCoord, gMatTransform).xy;

	return vout;
}