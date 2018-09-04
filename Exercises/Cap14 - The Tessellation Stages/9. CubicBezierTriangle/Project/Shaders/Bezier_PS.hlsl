//========================================================
// Bézier Tessellation PS
//========================================================

struct PixelIn
{
    float4 PosH : SV_POSITION;
};

float4 PS(PixelIn pin) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}