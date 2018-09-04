//========================================================
// Bézier Tessellation HS
//========================================================

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbPerPass : register(b2)
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
};

struct HullIn
{
    float3 PosL : POSITION;
};

struct HullOut
{
    float3 PosL : POSITION;
};

struct PatchTess
{
    float EdgeTess[4]   : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<HullIn, 16> patch, uint patchId : SV_PrimitiveID)
{
    PatchTess pt;

    // Uniformly tessellate the patch.

    pt.EdgeTess[0] = 25.0f;
    pt.EdgeTess[1] = 25.0f;
    pt.EdgeTess[2] = 25.0f;
    pt.EdgeTess[3] = 25.0f;

    pt.InsideTess[0] = 25.0f;
    pt.InsideTess[1] = 25.0f;

    return pt;
}

// This Hull Shader part is commonly used for a coordinate basis change,
// for example changing from a quad to a Bézier bi-cubic.
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(
    InputPatch<HullIn, 16> patch,
    uint index : SV_OutputControlPointID,
    uint patchId : SV_PrimitiveID)
{
    HullOut hout;

    hout.PosL = patch[index].PosL;

    return hout;
}