//========================================================
// Tessellation DS
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
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarz;
    float gTotalTime;
    float gDeltaTime;
};

struct DomainIn
{
    float3 PosL : POSITION;
};

struct DomainOut
{
    float4 PosH : SV_POSITION;
};

struct PatchTess
{
    float EdgeTess[4]   : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

// The domain shader is called for every vertex created by the tessellator.
// Is is like the vertex shader after tessellation.
[domain("quad")]
DomainOut DS(
    PatchTess patchTess,
    float2 uv : SV_DomainLocation,
    const OutputPatch<DomainIn, 4> quad)
{
    DomainOut dout;

    // Bilinear interpolation.
    float3 v1 = lerp(quad[0].PosL, quad[1].PosL, uv.x);
    float3 v2 = lerp(quad[2].PosL, quad[3].PosL, uv.x);
    float3 p  = lerp(v1, v2, uv.y);

    // Displacement mapping.
    p.y = 0.3f * (p.z * sin(p.x) + p.x * cos(p.z));

    float4 posW = mul(float4(p, 1.0f), gWorld);
    dout.PosH = mul(posW, gViewProj);

    return dout;
}