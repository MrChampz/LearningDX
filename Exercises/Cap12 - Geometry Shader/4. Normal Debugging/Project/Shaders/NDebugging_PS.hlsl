struct PixelIn
{
	float4 PosH : SV_POSITION;
};

float4 PS(PixelIn pin) : SV_TARGET
{
	return float4(1.0f, 1.0f, 0.0f, 1.0f);
}