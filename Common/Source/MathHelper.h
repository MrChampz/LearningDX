#pragma once

#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;

class MathHelper
{
public:
	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

public:
	// Returns random float in [0, 1] range.
	static float RandF();

	// Returns random float in [a, b] range.
	static float RandF(float a, float b);

	// Returns random int in [a, b] range.
	static int Rand(int a, int b);

	static XMVECTOR SphericalToCartesian(float radius, float theta, float phi);

	static XMFLOAT4X4 Identity4x4();

public:
	static const float Infinity;
	static const float Pi;
};
