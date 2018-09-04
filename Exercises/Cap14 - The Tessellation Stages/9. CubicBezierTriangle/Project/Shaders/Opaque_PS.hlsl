//========================================================
// Opaque PS
//========================================================

cbuffer cbPerPass : register(b1)
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
};

struct PixelIn
{
	float4 PosH : SV_POSITION;
};

float4 PS(PixelIn pin) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}