//========================================================
// Quadratic Bézier Surface Tessellation DS
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

float3 BernsteinBasis(float t)
{
    float invT = 1.0f - t;

    float b0 = invT * invT;      // B0^2(t) = (1-t)^2
    float b1 = 2.0f * t * invT;  // B1^2(t) = 2t(1-t)
    float b2 = t * t;            // B2^2(t) = t^2

    return float3(b0, b1, b2);
}

float3 QuadraticBezierSum(const OutputPatch<DomainIn, 9> bezPatch, float3 basisU, float3 basisV)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);

    sum += basisV.x * (basisU.x * bezPatch[0].PosL + basisU.y * bezPatch[1].PosL + basisU.z * bezPatch[2].PosL);
    sum += basisV.y * (basisU.x * bezPatch[3].PosL + basisU.y * bezPatch[4].PosL + basisU.z * bezPatch[5].PosL);
    sum += basisV.z * (basisU.x * bezPatch[6].PosL + basisU.y * bezPatch[7].PosL + basisU.z * bezPatch[8].PosL);

    return sum;
}

// The domain shader is called for every vertex created by the tessellator.
// Is is like the vertex shader after tessellation.
[domain("quad")]
DomainOut DS(
    PatchTess patchTess,
    float2 uv : SV_DomainLocation,
    const OutputPatch<DomainIn, 9> bezPatch)
{
    DomainOut dout;

    float3 basisU = BernsteinBasis(uv.x);
    float3 basisV = BernsteinBasis(uv.y);

    float3 p = QuadraticBezierSum(bezPatch, basisU, basisV);

    float4 posW = mul(float4(p, 1.0f), gWorld);
    dout.PosH = mul(posW, gViewProj);

    return dout;
}