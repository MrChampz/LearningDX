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
	static XMFLOAT4X4 Identity4x4();

public:
	static const float Infinity;
	static const float Pi;
};
