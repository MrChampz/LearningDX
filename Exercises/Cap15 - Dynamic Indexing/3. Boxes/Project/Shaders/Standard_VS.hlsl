// Include structures and functions for lighting.
#include "LightingUtil.hlsli"

struct MaterialData
{
    float4   DiffuseAlbedo;
    float3   FresnelR0;
    float    Roughness;
    float4x4 MatTransform;
};

// Put in space1, so the texture array does not overlap with these resources.
// The texture array will occupy registers t0, t1, ..., t3 in space0.
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
    uint     gMaterialIndex;
    uint     _ObjPad0;
    uint     _ObjPad1;
    uint     _ObjPad2;
};

cbuffer cbPass : register(b1)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewProj;
	float4x4 gInvView;
	float4x4 gInvProj;
	float4x4 gInvViewProj;
	float3   gEyePosW;
	float    _PassPad0;
	float2   gRenderTargetSize;
	float2   gInvRenderTargetSize;
	float    gNearZ;
	float    gFarz;
	float    gTotalTime;
	float    gDeltaTime;

	float4   gAmbientLight;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light    gLights[MaxLights];
};

struct VertexIn
{
	float3 PosL            : POSITION;
	float3 NormalL         : NORMAL;
	float2 TexCoord        : TEXCOORD;
    uint   DiffuseMapIndex : INDEX;
};

struct VertexOut
{
	float4 PosH	           : SV_POSITION;
	float3 PosW            : POSITION;
	float3 NormalW         : NORMAL;
	float2 TexCoord        : TEXCOORD;
    uint   DiffuseMapIndex : INDEX;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    // Fetch the material data.
    MaterialData matData = gMaterialData[gMaterialIndex];

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW = posW.xyz;

	// Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, gViewProj);

	// Output vertex attributes for interpolation across triangle.
	float4 texCoord = mul(float4(vin.TexCoord, 0.0f, 1.0f), gTexTransform);
	vout.TexCoord = mul(texCoord, matData.MatTransform).xy;

    // Pass the diffuse texture index to the pixel shader.
    vout.DiffuseMapIndex = vin.DiffuseMapIndex;

	return vout;
}