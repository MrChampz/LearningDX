#include "MathHelper.h"

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::RandF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

float MathHelper::RandF(float a, float b)
{
	return a + RandF() * (b - a);
}

int MathHelper::Rand(int a, int b)
{
	return a + rand() % ((b - a) + 1);
}

XMVECTOR MathHelper::SphericalToCartesian(float radius, float theta, float phi)
{
	return XMVectorSet
	(
		radius * sinf(phi) * cosf(theta),
		radius * cosf(phi),
		radius * sinf(phi) * sinf(theta),
		1.0f
	);
}

XMFLOAT4X4 MathHelper::Identity4x4()
{
	static XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}
