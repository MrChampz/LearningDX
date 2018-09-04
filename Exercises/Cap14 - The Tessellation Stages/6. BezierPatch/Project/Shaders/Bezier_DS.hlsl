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
    float EdgeTess[4]   : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;

    float b0 = invT * invT * invT;      // B0^3(t) = (1-t)^3
    float b1 = 3.0f * t * invT * invT;  // B1^3(t) = 3t(1-t)^2
    float b2 = 3.0f * t * t * invT;     // B2^3(t) = 3t^2(1-t)
    float b3 = t * t * t;               // B3^3(t) = t^3

    return float4(b0, b1, b2, b3);
}

float3 CubicBezierSum(const OutputPatch<DomainIn, 16> bezPatch, float4 basisU, float4 basisV)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);

    sum += basisV.x * (basisU.x * bezPatch[ 0].PosL + basisU.y * bezPatch[ 1].PosL + basisU.z * bezPatch[ 2].PosL + basisU.w * bezPatch[ 3].PosL);
    sum += basisV.y * (basisU.x * bezPatch[ 4].PosL + basisU.y * bezPatch[ 5].PosL + basisU.z * bezPatch[ 6].PosL + basisU.w * bezPatch[ 7].PosL);
    sum += basisV.z * (basisU.x * bezPatch[ 8].PosL + basisU.y * bezPatch[ 9].PosL + basisU.z * bezPatch[10].PosL + basisU.w * bezPatch[11].PosL);
    sum += basisV.w * (basisU.x * bezPatch[12].PosL + basisU.y * bezPatch[13].PosL + basisU.z * bezPatch[14].PosL + basisU.w * bezPatch[15].PosL);

    return sum;
}

// The domain shader is called for every vertex created by the tessellator.
// Is is like the vertex shader after tessellation.
[domain("quad")]
DomainOut DS(
    PatchTess patchTess,
    float2 uv : SV_DomainLocation,
    const OutputPatch<DomainIn, 16> bezPatch)
{
    DomainOut dout;

    float4 basisU = BernsteinBasis(uv.x);
    float4 basisV = BernsteinBasis(uv.y);

    float3 p = CubicBezierSum(bezPatch, basisU, basisV);

    float4 posW = mul(float4(p, 1.0f), gWorld);
    dout.PosH = mul(posW, gViewProj);

    return dout;
}