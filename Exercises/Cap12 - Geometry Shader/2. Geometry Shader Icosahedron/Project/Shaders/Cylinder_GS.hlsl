// Include structures and functions for lighting.
#include "LightingUtil.hlsli"

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
};

cbuffer cbPass : register(b2)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewProj;
	float4x4 gInvView;
	float4x4 gInvProj;
	float4x4 gInvViewProj;
	float3   gEyePosW;
	float    cbPerObjectPad1;
	float2   gRenderTargetSize;
	float2   gInvRenderTargetSize;
	float    gNearZ;
	float    gFarz;
	float    gTotalTime;
	float    gDeltaTime;

	float4 gAmbientLight;

	// Allow application to change fog parameters once per frame.
	// For example, we may only use fog for certain times of day.
	float4 gFogColor;
	float  gFogStart;
	float  gFogRange;
	float2 cbPerObjectPad2;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
	Light gLights[MaxLights];
};

struct GeoIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct GeoOut
{
	float4 PosH	    : SV_POSITION;
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
	float2 TexCoord : TEXCOORD;
};

void Subdivide(GeoIn inVerts[3], out GeoIn outVerts[6])
{
	//       1
	//       *
	//      / \
	//     /   \
	//   m0*–––*m1
	//   / \   / \
	//  /   \ /   \
	// *–—*–—*–—*–—*
	// 0     m2    2
	GeoIn m[3];

	// Compute edge midpoints.
	m[0].PosL = 0.5f * (inVerts[0].PosL + inVerts[1].PosL);
	m[1].PosL = 0.5f * (inVerts[1].PosL + inVerts[2].PosL);
	m[2].PosL = 0.5f * (inVerts[2].PosL + inVerts[0].PosL);

	// Project onto unit sphere.
	m[0].PosL = normalize(m[0].PosL);
	m[1].PosL = normalize(m[1].PosL);
	m[2].PosL = normalize(m[2].PosL);

	// Remap to the radius.
	m[0].PosL = m[0].PosL * 5.0f;
	m[1].PosL = m[1].PosL * 5.0f;
	m[2].PosL = m[2].PosL * 5.0f;

	// Derive normals.
	m[0].NormalL = m[0].PosL;
	m[1].NormalL = m[1].PosL;
	m[2].NormalL = m[2].PosL;

	// Interpolate texture coordinates.
	m[0].TexCoord = 0.5f * (inVerts[0].TexCoord + inVerts[1].TexCoord);
	m[1].TexCoord = 0.5f * (inVerts[1].TexCoord + inVerts[2].TexCoord);
	m[2].TexCoord = 0.5f * (inVerts[2].TexCoord + inVerts[0].TexCoord);

	outVerts[0] = inVerts[0];
	outVerts[1] = m[0];
	outVerts[2] = m[2];
	outVerts[3] = m[1];
	outVerts[4] = inVerts[2];
	outVerts[5] = inVerts[1];
}

void OutputSubdivision(GeoIn v[6], inout TriangleStream<GeoOut> stream)
{
	GeoOut gout[6];

	[unroll]
	for (int i = 0; i < 6; ++i)
	{
		// Transform to world space.
		gout[i].PosW = mul(float4(v[i].PosL, 1.0f), gWorld).xyz;
		gout[i].NormalW = mul(v[i].NormalL, (float3x3)gWorld);

		// Transform to homogeneous clip space.
		gout[i].PosH = mul(float4(gout[i].PosW, 1.0f), gViewProj);

		gout[i].TexCoord = v[i].TexCoord;
	}

	//       1
	//       *
	//      / \
	//     /   \
	//   m0*–––*m1
	//   / \   / \
	//  /   \ /   \
	// *–—*–—*–—*–—*
	// 0     m2    2

	// We can draw the subdivision in two strips:
	//	Strip 1: bottom three triangles
	//	Strip 2: top triangle

	[unroll]
	for (int i = 0; i < 5; ++i)
	{
		stream.Append(gout[i]);
	}

	stream.RestartStrip();
	stream.Append(gout[1]);
	stream.Append(gout[5]);
	stream.Append(gout[3]);
}

void SubdivideSubtriangles(GeoIn inVerts[6], inout TriangleStream<GeoOut> stream)
{
	GeoIn v[6];
	GeoIn s[3];

	//       
	//       *
	//      / \
	//     /   \
	//  1 *–––––*
	//   / \   / \
	//  /   \ /   \
	// *–—*–—*–—*–—*
	// 0     2    
	s[0] = inVerts[0];
	s[1] = inVerts[1];
	s[2] = inVerts[2];

	Subdivide(s, v);
	OutputSubdivision(v, stream);

	//       
	//       *
	//      / \
	//     /   \
	//  1 *–––––* 3
	//   / \   / \
	//  /   \ /   \
	// *–—*–—*–—*–—*
	//       2    
	s[0] = inVerts[1];
	s[1] = inVerts[3];
	s[2] = inVerts[2];

	Subdivide(s, v);
	OutputSubdivision(v, stream);

	//       
	//       *
	//      / \
	//     /   \
	//    *–––––* 3
	//   / \   / \
	//  /   \ /   \
	// *–—*–—*–—*–—*
	//       2     4
	s[0] = inVerts[2];
	s[1] = inVerts[3];
	s[2] = inVerts[4];

	Subdivide(s, v);
	OutputSubdivision(v, stream);

	//       5
	//       *
	//      / \
	//     /   \
	//  1 *–––––* 3
	//   / \   / \
	//  /   \ /   \
	// *–—*–—*–—*–—*
	//        
	s[0] = inVerts[1];
	s[1] = inVerts[5];
	s[2] = inVerts[3];

	Subdivide(s, v);
	OutputSubdivision(v, stream);
}

[maxvertexcount(64)]
void GS(triangle GeoIn gin[3], inout TriangleStream<GeoOut> stream)
{
	//GeoIn v[6];
	//Subdivide(gin, v);
	//OutputSubdivision(v, stream);
	//SubdivideSubtriangles(v, stream);

	// Get the distance between icosahedron (at world origin) and camera.
	float  d = length(gEyePosW);

	int subdivs;

	if (d < 15)
		subdivs = 2;
	else if (d >= 15 && d < 30)
		subdivs = 1;
	else if (d >= 30)
		subdivs = 0;

	if (subdivs > 0)
	{
		gin[0].PosL = normalize(gin[0].PosL);
		gin[1].PosL = normalize(gin[1].PosL);
		gin[2].PosL = normalize(gin[2].PosL);

		uint level_count = (uint)pow(2, subdivs);
		float3 v2 = (gin[1].PosL - gin[0].PosL) / level_count,
			   v1 = (gin[2].PosL - gin[0].PosL) / level_count,
			   v0 =  gin[0].PosL;

		float3 nu = gin[1].PosL;
		float3 nw = gin[2].PosL;
		float3 nv = gin[0].PosL;

		float2 tu = (gin[1].TexCoord - gin[0].TexCoord) / level_count,
			   tw = (gin[2].TexCoord - gin[0].TexCoord) / level_count,
			   tv =  gin[0].TexCoord;

		for (int level = level_count; level > 0; --level)
		{
			GeoOut gout;

			float3 ab = normalize(v0) * 5.0f;

			// Transform to world space.
			gout.PosW = mul(float4(ab, 1.0f), gWorld).xyz;
			gout.NormalW = mul(nv, (float3x3)gWorld);

			// Transform to homogeneous clip space.
			gout.PosH = mul(float4(gout.PosW, 1.0f), gViewProj);

			gout.TexCoord = tv;

			stream.Append(gout);

			float3 p = v0;
			uint   vertex_count = level + 1;

			for (uint vertex = 1; vertex < vertex_count; ++vertex)
			{
				// Transform to world space.
				float3 a1 = normalize(p + v2) * 5.0f;
				gout.PosW = mul(float4(a1, 1.0f), gWorld).xyz;
				gout.NormalW = mul(nu, (float3x3)gWorld);

				// Transform to homogeneous clip space.
				gout.PosH = mul(float4(gout.PosW, 1.0f), gViewProj);

				gout.TexCoord = tu;

				stream.Append(gout);

				// Transform to world space.
				float3 a2 = normalize(p += v1) * 5.0f;
				gout.PosW = mul(float4(a2, 1.0f), gWorld).xyz;
				gout.NormalW = mul(nw, (float3x3)gWorld);

				// Transform to homogeneous clip space.
				gout.PosH = mul(float4(gout.PosW, 1.0f), gViewProj);

				gout.TexCoord = tw;

				stream.Append(gout);
			}

			v0 += v2;
			stream.RestartStrip();
		}
	}
	else
	{
		[unroll]
		for (uint i = 0; i < 3; ++i)
		{
			GeoOut gout;

			// Transform to world space.
			gout.PosW = mul(float4(gin[i].PosL, 1.0f), gWorld).xyz;
			gout.NormalW = mul(gin[i].NormalL, (float3x3)gWorld);

			// Transform to homogeneous clip space.
			gout.PosH = mul(float4(gout.PosW, 1.0f), gViewProj);

			gout.TexCoord = gin[i].TexCoord;

			stream.Append(gout);
		}
	}
}