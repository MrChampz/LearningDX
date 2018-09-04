/**
 * Performs a separable Gaussian blur with a blur radius up to 5 pixels.
 */

cbuffer cbSettings : register(b0)
{
	// We cannot have an array entry in a constant buffer that gets mapped onto
	// root constants, so list each element.

	int gBlurRadius;

	// Support up to 11 blur weights.
	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};

static const int gMaxBlurRadius = 5;

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 256
#define CacheSize (N + 2 * gMaxBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 GTid : SV_GroupThreadID, int3 DTid : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth. To blur
	// N pixels, we will need to load N + 2 * BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads. To get the extra 2 * BlurRadius pixels,
	// have 2 * BlurRadius threads sample an extra pixel.
	if (GTid.x < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = max(DTid.x - gBlurRadius, 0);
		gCache[GTid.x] = gInput[int2(x, DTid.y)];
	}
	if (GTid.x >= N - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = min(DTid.x + gBlurRadius, gInput.Length.x - 1);
		gCache[GTid.x + 2 * gBlurRadius] = gInput[int2(x, DTid.y)];
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[GTid.x + gBlurRadius] = gInput[min(DTid.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = GTid.x + gBlurRadius + i;

		blurColor += weights[i + gBlurRadius] * gCache[k];
	}

	gOutput[DTid.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 GTid : SV_GroupThreadID, int3 DTid : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth. To blur
	// N pixels, we will need to load N + 2 * BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads. To get the extra 2 * BlurRadius pixels,
	// have 2 * BlurRadius threads sample an extra pixel.
	if (GTid.y < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = max(DTid.y - gBlurRadius, 0);
		gCache[GTid.y] = gInput[int2(DTid.x, y)];
	}
	if (GTid.y >= N - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = min(DTid.y + gBlurRadius, gInput.Length.y - 1);
		gCache[GTid.y + 2 * gBlurRadius] = gInput[int2(DTid.x, y)];
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[GTid.y + gBlurRadius] = gInput[min(DTid.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = GTid.y + gBlurRadius + i;

		blurColor += weights[i + gBlurRadius] * gCache[k];
	}

	gOutput[DTid.xy] = blurColor;
}