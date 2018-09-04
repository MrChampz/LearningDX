//=======================================================================
// Combines two images.
//=======================================================================

Texture2D gBaseMap : register(t0);
Texture2D gEdgeMap : register(t1);

SamplerState gSamplerPointWrap        : register(s0);
SamplerState gSamplerPointClamp       : register(s1);
SamplerState gSamplerLinearWrap       : register(s2);
SamplerState gSamplerLinearClamp	  : register(s3);
SamplerState gSamplerAnisotropicWrap  : register(s4);
SamplerState gSamplerAnisotropicClamp : register(s5);

struct PixelIn
{
    float4 PosH     : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 PS(PixelIn pin) : SV_Target
{
    float4 c = gBaseMap.SampleLevel(gSamplerPointClamp, pin.TexCoord, 0.0f);
    float4 e = gEdgeMap.SampleLevel(gSamplerPointClamp, pin.TexCoord, 0.0f);

    return c * e;
}