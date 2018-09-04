//========================================================
// Bézier Tessellation DS
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
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess  : SV_InsideTessFactor;
};

struct Bernstein
{
    float B[10];
};

Bernstein BernsteinBasis(float3 uvw)
{
    Bernstein b;

    b.B[0] = uvw.x * uvw.x * uvw.x;         // B300^3(u,v,w) = u^3
    b.B[1] = 3.0f * uvw.x * uvw.x * uvw.y;  // B210^3(u,v,w) = 3u^2v
    b.B[2] = 3.0f * uvw.x * uvw.y * uvw.y;  // B120^3(u,v,w) = 3uv^2
    b.B[3] = uvw.y * uvw.y * uvw.y;         // B030^3(u,v,w) = v^3
    b.B[4] = 3.0f * uvw.x * uvw.x * uvw.z;  // B201^3(u,v,w) = 3u^2w
    b.B[5] = 6.0f * uvw.x * uvw.y * uvw.z;  // B111^3(u,v,w) = 6uvw
    b.B[6] = 3.0f * uvw.y * uvw.y * uvw.z;  // B021^3(u,v,w) = 3v^2w
    b.B[7] = 3.0f * uvw.x * uvw.z * uvw.z;  // B102^3(u,v,w) = 3uw^2
    b.B[8] = 3.0f * uvw.y * uvw.z * uvw.z;  // B012^3(u,v,w) = 3vw^2
    b.B[9] = uvw.z * uvw.z * uvw.z;         // B003^3(u,v,w) = w^3

    return b;
}

float3 CubicBezierSum(const OutputPatch<DomainIn, 10> bezPatch, Bernstein basis)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);

    sum += basis.B[0] * bezPatch[9].PosL;
    sum += basis.B[1] * bezPatch[7].PosL;
    sum += basis.B[2] * bezPatch[4].PosL;
    sum += basis.B[3] * bezPatch[0].PosL;
    sum += basis.B[4] * bezPatch[8].PosL;
    sum += basis.B[5] * bezPatch[5].PosL;
    sum += basis.B[6] * bezPatch[1].PosL;
    sum += basis.B[7] * bezPatch[6].PosL;
    sum += basis.B[8] * bezPatch[2].PosL;
    sum += basis.B[9] * bezPatch[3].PosL;

    return sum;
}

// The domain shader is called for every vertex created by the tessellator.
// Is is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DS(
    PatchTess patchTess,
    float3 uvw : SV_DomainLocation,
    const OutputPatch<DomainIn, 10> bezPatch)
{
    DomainOut dout;

    Bernstein basis = BernsteinBasis(uvw);

    float3 p = CubicBezierSum(bezPatch, basis);

    float4 posW = mul(float4(p, 1.0f), gWorld);
    dout.PosH = mul(posW, gViewProj);

    return dout;
}