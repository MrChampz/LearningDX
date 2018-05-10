#pragma once

#include <Windows.h>
#include <DirectXMath.h>
using namespace DirectX;

class MathHelper
{
public:
	static XMFLOAT4X4 Identity4x4();

public:
	static const float Infinity;
	static const float Pi;
};
