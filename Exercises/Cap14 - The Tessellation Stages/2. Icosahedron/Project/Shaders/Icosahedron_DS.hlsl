//========================================================
// Icosahedron DS
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
    float    cbPerObjectPad1;
    float2   gRenderTargetSize;
    float2   gInvRenderTargetSize;
    float    gNearZ;
    float    gFarz;
    float    gTotalTime;
    float    gDeltaTime;
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
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess  : SV_InsideTessFactor;
};

// The domain shader is called for every vertex created by the tessellator.
// Is is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DS(
    PatchTess patchTess,
    float3 uvw : SV_DomainLocation,
    const OutputPatch<DomainIn, 3> patch)
{
    DomainOut dout;

    // Bilinear interpolation.
    float3 p = float3(0.0f, 0.0f, 0.0f);
    p += patch[0].PosL * uvw.x;
    p += patch[1].PosL * uvw.y;
    p += patch[2].PosL * uvw.z;

    // Project p onto unit sphere.
    p = normalize(p);

    // Scale p by the radius (5.0f).
    p = p * 5.0f;

    float4 posW = mul(float4(p, 1.0f), gWorld);
    dout.PosH = mul(posW, gViewProj);

    return dout;
}