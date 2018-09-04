/**
 * Performs a separable Bilateral Filtering with a radius up to 5 pixels.
 */

cbuffer cbSettings : register(b0)
{
	// We cannot have an array entry in a constant buffer that gets mapped onto
	// root constants, so list each element.

	int gBlurRadius;

    float gSigmaR;

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

float CalcGaussRangeWeight(float difference)
{
    return exp(-(difference * difference) / (2.0f * (gSigmaR * gSigmaR)));
}

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 GTid : SV_GroupThreadID, int3 DTid : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float weights_s[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    float2 inputLength;
    gInput.GetDimensions(inputLength.x, inputLength.y);

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
        int x = min(DTid.x + gBlurRadius, inputLength.x - 1);
		gCache[GTid.x + 2 * gBlurRadius] = gInput[int2(x, DTid.y)];
	}

	// Clamp out of bound samples that occur at image borders.
    gCache[GTid.x + gBlurRadius] = gInput[min(DTid.xy, inputLength - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
    
    //
    // Compute the range sigma term.
    //

    float weights_r[11];
    for (int j = -gBlurRadius; j <= gBlurRadius; ++j)
    {
        int r = GTid.x + gBlurRadius;
        int k = GTid.x + gBlurRadius + j;
        
        float diff = length(distance(gCache[r], gCache[k]));

        weights_r[j + gBlurRadius] = CalcGaussRangeWeight(diff);
    }

	//
	// Now blur each pixel.
	//

    float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float  totalWeight = 0.0f;

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
        int k = GTid.x + gBlurRadius + i;

        blurColor += weights_s[i + gBlurRadius] * weights_r[i + gBlurRadius] * gCache[k];
        totalWeight += weights_s[i + gBlurRadius] * weights_r[i + gBlurRadius];
    }

    blurColor = (1.0f / totalWeight) * blurColor;

	gOutput[DTid.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 GTid : SV_GroupThreadID, int3 DTid : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float weights_s[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    float2 inputLength;
    gInput.GetDimensions(inputLength.x, inputLength.y);

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
        int y = min(DTid.y + gBlurRadius, inputLength.y - 1);
		gCache[GTid.y + 2 * gBlurRadius] = gInput[int2(DTid.x, y)];
	}

	// Clamp out of bound samples that occur at image borders.
    gCache[GTid.y + gBlurRadius] = gInput[min(DTid.xy, inputLength - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

    //
    // Compute the range sigma term.
    //

    float weights_r[11];
    for (int j = -gBlurRadius; j <= gBlurRadius; ++j)
    {
        int r = GTid.y + gBlurRadius;
        int k = GTid.y + gBlurRadius + j;
        
        float diff = length(distance(gCache[r], gCache[k]));

        weights_r[j + gBlurRadius] = CalcGaussRangeWeight(diff);
    }

	//
	// Now blur each pixel.
	//

    float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float  totalWeight = 0.0f;

    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = GTid.y + gBlurRadius + i;

        blurColor += weights_s[i + gBlurRadius] * weights_r[i + gBlurRadius] * gCache[k];
        totalWeight += weights_s[i + gBlurRadius] * weights_r[i + gBlurRadius];
    }

    blurColor = (1.0f / totalWeight) * blurColor;

	gOutput[DTid.xy] = blurColor;
}