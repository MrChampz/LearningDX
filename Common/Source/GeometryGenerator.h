//***************************************************************************************
// GeometryGenerator.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines a static class for procedurally generating the geometry of 
// common mathematical objects.
//
// All triangles are generated "outward" facing.  If you want "inward" 
// facing triangles (for example, if you want to place the camera inside
// a sphere to simulate a sky), you will need to:
//   1. Change the Direct3D cull mode or manually reverse the winding order.
//   2. Invert the normal.
//   3. Update the texture coordinates and tangent vectors.
//***************************************************************************************
#pragma once

#include <cstdint>
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

using uint16 = std::uint16_t;
using uint32 = std::uint32_t;

class GeometryGenerator
{
public:
	struct Vertex
	{
		Vertex() { }

		Vertex(
			const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexCoord(uv) {}
		
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexCoord(u, v) { }

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexCoord;
	};

	struct MeshData
	{
	public:
		std::vector<uint16>& GetIndices16()
		{
			if (m_Indices16.empty())
			{
				m_Indices16.resize(Indices32.size());
				for (size_t i = 0; i < Indices32.size(); ++i)
				{
					m_Indices16[i] = static_cast<uint16>(Indices32[i]);
				}
			}

			return m_Indices16;
		}

	public:
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices32;

	private:
		std::vector<uint16> m_Indices16;
	};

	/// Creates a box centered at the origin with the given dimensions, where each
	/// face has m rows and n columns of vertices.
	static MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

	/// Creates a sphere centered at the origin with the given radius. The
	/// slices and stacks parameters control the degree of tessellation.
	static MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

	/// Creates a cylinder parallel to the y-axis, and centered about the origin.
	/// The bottom and top radius can vary to form various cone shapes rather than true
	/// cylinders. The slice and stacks parameters control the degree of tessellation.
	static MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

	/// Creates an m x n grid in the xz-plane with m rows and n columns, centered
	/// at the origin with the specified width and depth.
	static MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);

private:
	static void Subdivide(MeshData& meshData);
	static void BuildCylinderTopCap(float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	static void BuildCylinderBottomCap(float bottomRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
};