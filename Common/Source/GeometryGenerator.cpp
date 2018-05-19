#include "GeometryGenerator.h"

#include <algorithm>

GeometryGenerator::MeshData
GeometryGenerator::CreateBox(float width, float height, float depth, uint32 numSubdivisions)
{
	MeshData meshData;

	// 
	// Create the vertices.
	//

	Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8]  = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9]  = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint32 i[36];

	// Fill in the front face index data.
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data.
	i[6] = 4; i[7]  = 5; i[8]  = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data.
	i[12] = 8; i[13] =  9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data.
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data.
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data.
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices32.assign(&i[0], &i[36]);

	// Put a cap on the number of subdivisions.
	numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

	for (uint32 i = 0; i < numSubdivisions; ++i)
	{
		Subdivide(meshData);
	}

	return meshData;
}

GeometryGenerator::MeshData
GeometryGenerator::CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
{
	MeshData meshData;

	//
	// Compute the vertices starting at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back(topVertex);

	// stackCount = 3
	// sliceCount = 4
	// radius 5

	float phiStep = XM_PI / stackCount; // phi = meia circunferencia na vertical
	float thetaStep = 2.0f * XM_PI / sliceCount; // theta = circunferencia na horizonral (dá a volta na esfera)

	// phiStep = 1.047197580 (60°)
	// thetaStep = 1.5707963705 (90°)

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32 i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// phi = 1 * 1.047197580 = 1.047197580	(60°)
		// phi = 2 * 1.047197580 = 2.094395160	(120°)

		// Vertices of ring.
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			// theta = 0 * 1.5707963705 = 0				(0°)
			// theta = 1 * 1.5707963705 = 1.5707963705	(90°)
			// theta = 2 * 1.5707963705 = 3.1415927410	(180°)
			// theta = 3 * 1.5707963705 = 4.7123891115	(270°)
			// theta = 4 * 1.5707963705 = 6.2831854820	(360°)

			Vertex v;

			// Spherical to cartesian.
			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);

			// phi = 60°
			// theta = 0°
			// position.x = 5 * sin(1.047197580) * cos(0)             =  4.33012709093
			// position.y = 5 * cos(1.047197580)                      =  2.5
			// position.z = 5 * sin(1.047197580) * sin(0)             =  0

			// phi = 60°
			// theta = 90°
			// position.x = 5 * sin(1.047197580) * cos(1.5707963705)  =  0
			// position.y = 5 * cos(1.047197580)					  =  2.5
			// position.z = 5 * sin(1.047197580) * sin(1.5707963705)  =  4.33012701892

			// phi = 60°
			// theta = 180°
			// position.x = 5 * sin(1.047197580) * cos(3.1415927410)  = -4.33012701892
			// position.y = 5 * cos(1.047197580)					  =  2.5
			// position.z = 5 * sin(1.047197580) * sin(3.1415927410)  =  0

			// phi = 60°
			// theta = 270°
			// position.x = 5 * sin(1.047197580) * cos(4.7123891115)  =  0
			// position.y = 5 * cos(1.047197580)					  =  2.5
			// position.z = 5 * sin(1.047197580) * sin(4.7123891115)  = -4.33012701892

			// phi = 60°
			// theta = 360°(0°)
			// position.x = 5 * sin(1.047197580) * cos(6.2831854820)  =  4.33012709093
			// position.y = 5 * cos(1.047197580)					  =  2.5
			// position.z = 5 * sin(1.047197580) * sin(6.2831854820)  =  0




			// phi = 120°
			// theta = 0°
			// position.x = 5 * sin(2.094395160) * cos(0)             =  4.33012701892
			// position.y = 5 * cos(2.094395160)                      = -2.5
			// position.z = 5 * sin(2.094395160) * sin(0)             =  0

			// phi = 120°
			// theta = 90°
			// position.x = 5 * sin(2.094395160) * cos(1.5707963705)  =  0
			// position.y = 5 * cos(2.094395160)					  = -2.5
			// position.z = 5 * sin(2.094395160) * sin(1.5707963705)  =  4.33012701892

			// phi = 120°
			// theta = 180°
			// position.x = 5 * sin(2.094395160) * cos(3.1415927410)  = -4.33012701892
			// position.y = 5 * cos(2.094395160)					  = -2.5
			// position.z = 5 * sin(2.094395160) * sin(3.1415927410)  =  0

			// phi = 120°
			// theta = 270°
			// position.x = 5 * sin(2.094395160) * cos(4.7123891115)  =  0
			// position.y = 5 * cos(2.094395160)					  = -2.5
			// position.z = 5 * sin(2.094395160) * sin(4.7123891115)  = -4.33012701892

			// phi = 120°
			// theta = 360°(0°)
			// position.x = 5 * sin(2.094395160) * cos(6.2831854820)  =  4.33012709093
			// position.y = 5 * cos(2.094395160)					  = -2.5
			// position.z = 5 * sin(2.094395160) * sin(6.2831854820)  =  0

			// Partial derivative of P with respect to theta.
			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y =  0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&v.TangentU);
			XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

			XMVECTOR P = XMLoadFloat3(&v.Position);
			XMStoreFloat3(&v.Normal, XMVector3Normalize(P));

			v.TexCoord.x = theta / XM_2PI;
			v.TexCoord.y = phi / XM_PI;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack. The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32 i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices32.push_back(0);
		meshData.Indices32.push_back(i + 1);
		meshData.Indices32.push_back(i);

		// indices = 0
		// indices = 2
		// indices = 1

		// indices = 0
		// indices = 3
		// indices = 2

		// indices = 0
		// indices = 4
		// indices = 3

		// indices = 0
		// indices = 5
		// indices = 4
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32 baseIndex = 1;
	uint32 ringVertexCount = sliceCount + 1;
	for (uint32 i = 0; i < stackCount - 2; ++i)
	{
		for (uint32 j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	// 
	// Compute indices for bottom stack. The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32 southPoleIndex = (uint32)meshData.Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(southPoleIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);

		// indices = 11
		// indices = 6
		// indices = 7

		// indices = 11
		// indices = 7
		// indices = 8

		// indices = 11
		// indices = 8
		// indices = 9

		// indices = 11
		// indices = 9
		// indices = 10
	}

	return meshData;
}

GeometryGenerator::MeshData
GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
{
	MeshData meshData;

	//
	// Build stacks.
	//

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	uint32 ringCount = stackCount + 1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (uint32 i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// bottomRadius = 5
		// topRadius    = 2
		// height       = 10
		// sliceCount   = 4
		// stackCount   = 3

		// stackHeight = height  /  stackCount
		// stackHeight =   10    /      3        = 3.333..

		// radiusStep = (topRadius - bottomRadius)  /  stackCount
		// radiusStep = (    2     -      5      )  /      3        = -1

		// ringCount = stackCount  +  1
		// ringCount =     3       +  1   = 4

		// y = -0.5  *  height  +  i  *  stackHeight
		// y = -0.5  *    10    +  0  *     3.333      = -5
		// y = -0.5  *    10    +  1  *     3.333      = -1,667
		// y = -0.5  *    10    +  2  *     3.333      = +1,666
		// y = -0.5  *    10    +  3  *     3.333      = +4,999

		// r = bottomRadius  +  i  *  radiusStep
		// r =      5        +  0  *      -1           = 5
		// r =      5        +  1  *      -1           = 4
		// r =      5        +  2  *      -1           = 3
		// r =      5        +  3  *      -1           = 2

		// Vertices of ring.
		float dTheta = 2.0f * XM_PI / sliceCount;
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			// PI equivale a metade da circunferencia, 180°, logo 2 * PI
			// equivale a 360°. 360° / sliceCount resulta no comprimento
			// da circunferencia de cada "pedaço" (slice). 360° / 4 = 90° = 1.5708 radianos
			//
			// dTheta = 2.0f  *     XM_PI     /  sliceCount
			// dTheta = 2.0f  *  3.141592741  /      4        = 1.5707963705

			Vertex vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.Position = XMFLOAT3(r * c, y, r * s);

			// c = cos( j  *     dTheta    )
			// c = cos( 0  *  1.5707963705 )  = cos(0)            =  1
			// c = cos( 1  *  1.5707963705 )  = cos(1.5707963705) =  0
			// c = cos( 2  *  1.5707963705 )  = cos(3.1415927410) = -1
			// c = cos( 3  *  1.5707963705 )  = cos(4.7123891115) =  0
			// c = cos( 4  *  1.5707963705 )  = cos(6.283185482 ) =  1

			// s = sin( j  *     dTheta    )
			// s = sin( 0  *  1.5707963705 )  = sin(0)            =  0
			// s = sin( 1  *  1.5707963705 )  = sin(1.5707963705) =  1
			// s = sin( 2  *  1.5707963705 )  = sin(3.1415927410) =  0
			// s = sin( 3  *  1.5707963705 )  = sin(4.7123891115) = -1
			// s = sin( 4  *  1.5707963705 )  = sin(6.283185482 ) =  0

			// Vértices apenas do 1° anel!
			// vertex.Position = XMFLOAT3( r  *  c,  y,  r  *  s )
			// vertex.Position = XMFLOAT3( 5  *  1, -5,  5  *  0 )  =  (  5, -5,  0 )
			// vertex.Position = XMFLOAT3( 5  *  0, -5,  5  *  1 )  =  (  0, -5,  5 )
			// vertex.Position = XMFLOAT3( 5  * -1, -5,  5  *  0 )  =  ( -5, -5,  0 )
			// vertex.Position = XMFLOAT3( 5  *  0, -5,  5  * -1 )  =  (  0, -5, -5 )
			// vertex.Position = XMFLOAT3( 5  *  1, -5,  5  *  0 )  =  (  5, -5,  0 )

			vertex.TexCoord.x = (float)j / sliceCount;
			vertex.TexCoord.y = 1.0f - (float)i / stackCount;

			// texcoord.x = 0 / 4 = 0
			// texcoord.x = 1 / 4 = 0.25
			// texcoord.x = 2 / 4 = 0.50
			// texcoord.x = 3 / 4 = 0.75

			// texcoord.y = 1.0 - 0 / 3 = 1
			// texcoord.y = 1.0 - 1 / 3 = 0.6666
			// texcoord.y = 1.0 - 2 / 3 = 0.3333

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//
			// Let r0 be the bottom radius and let r1 be the top radius.
			// y(v) = h - hv for v in [0,1].
			// r(v) = r1 + (r0 - r1)v
			//
			// x(t, v) = r(v) * cos(t)
			// y(t, v) = h - hv
			// z(t, v) = r(v) * sin(t)
			// 
			// dx / dt = -r(v) * sin(t)
			// dy / dt = 0
			// dz / dt = +r(v) * cos(t)
			//
			// dx / dv = (r0 - r1) * cos(t)
			// dy / dv = -h
			// dz / dv = (r0 - r1) * sin(t)

			// This is unit length.
			vertex.TangentU = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr * c, -height, dr * s);

			XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
			XMVECTOR B = XMLoadFloat3(&bitangent);
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
			XMStoreFloat3(&vertex.Normal, N);

			meshData.Vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	uint32 ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (uint32 i = 0; i < stackCount; ++i)
	{
		for (uint32 j = 0; j < sliceCount; ++j)
		{
			meshData.Indices32.push_back(i * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);

			meshData.Indices32.push_back(i * ringVertexCount + j);
			meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.Indices32.push_back(i * ringVertexCount + j + 1);

			// ringVertexCount = 5
			// [0, 5, 6]
			// [0, 6, 1]
			//
			// 0 * 5 + 0 = 0
			// (0 + 1) * 5 + 0 = 5
			// (0 + 1) * 5 + 0 + 1 = 6
			// 0 * 5 + 0 = 0
			// (0 + 1) * 5 + 0 + 1 = 6
			// 0 * 5 + 0 + 1 = 1

			// [1, 6, 7]
			// [1, 7, 2]
			//
			// 0 * 5 + 1 = 1
			// (0 + 1) * 5 + 1 = 6
			// (0 + 1) * 5 + 1 + 1 = 7
			// 0 * 5 + 1 = 1
			// (0 + 1) * 5 + 1 + 1 = 7
			// 0 * 5 + 1 + 1 = 2
		}
	}

	BuildCylinderTopCap(topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, height, sliceCount, stackCount, meshData);

	return meshData;
}

GeometryGenerator::MeshData
GeometryGenerator::CreateGrid(float width, float depth, uint32 m, uint32 n)
{
	MeshData meshData;

	uint32 vertexCount = m * n;
	uint32 faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.Vertices.resize(vertexCount);
	for (uint32 i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (uint32 j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i * n + j].Position = XMFLOAT3(x, 0.0f, z);
			meshData.Vertices[i * n + j].Normal   = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * n + j].TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[i * n + j].TexCoord.x = j * du;
			meshData.Vertices[i * n + j].TexCoord.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	meshData.Indices32.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32 k = 0;
	for (uint32 i = 0; i < m - 1; ++i)
	{
		for (uint32 j = 0; j < n - 1; ++j)
		{
			meshData.Indices32[k + 0] = i * n + j;
			meshData.Indices32[k + 1] = i * n + j + 1;
			meshData.Indices32[k + 2] = (i + 1) * n + j;

			meshData.Indices32[k + 3] = (i + 1) * n + j;
			meshData.Indices32[k + 4] = i * n + j + 1;
			meshData.Indices32[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	return meshData;
}

void GeometryGenerator::Subdivide(MeshData& meshData)
{

}

void GeometryGenerator::BuildCylinderTopCap(float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
	// 
	// Build top cap.
	//

	uint32 baseIndex = (uint32)meshData.Vertices.size();

	float y = 0.5f * height;

	// y = 0.5f * height
	// y = 0.5f *   10    = 5.0f;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	float dTheta = 2.0f * XM_PI / sliceCount;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		// PI equivale a metade da circunferencia, 180°, logo 2 * PI
		// equivale a 360°. 360° / sliceCount resulta no comprimento
		// da circunferencia de cada "pedaço" (slice). 360° / 4 = 90° = 1.5708 radianos
		//
		// dTheta = 2.0f  *     XM_PI     /  sliceCount
		// dTheta = 2.0f  *  3.141592741  /      4        = 1.5707963705

		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// x = topRadius  *  cos( i  *     dTheta    )
		// x =    2       *  cos( 0  *  1.5707963705 )  = 2 * cos(0)            =  2
		// x =    2       *  cos( 1  *  1.5707963705 )  = 2 * cos(1.5707963705) =  0
		// x =    2       *  cos( 2  *  1.5707963705 )  = 2 * cos(3.1415927410) = -2
		// x =    2       *  cos( 3  *  1.5707963705 )  = 2 * cos(4.7123891115) =  0
		// x =    2       *  cos( 4  *  1.5707963705 )  = 2 * cos(6.283185482 ) =  2

		// z = topRadius  *  sin( i  *     dTheta    )
		// z =    2       *  sin( 0  *  1.5707963705 )  = 2 * sin(0)            =  0
		// z =    2       *  sin( 1  *  1.5707963705 )  = 2 * sin(1.5707963705) =  2
		// z =    2       *  sin( 2  *  1.5707963705 )  = 2 * sin(3.1415927410) =  0
		// z =    2       *  sin( 3  *  1.5707963705 )  = 2 * sin(4.7123891115) = -2
		// z =    2       *  sin( 4  *  1.5707963705 )  = 2 * sin(6.283185482 ) =  0

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));

		// vertice = ( 2.0f, 5.0f,  0.0f)
		// vertice = ( 0.0f, 5.0f,  2.0f)
		// vertice = (-2.0f, 5.0f,  0.0f)
		// vertice = ( 0.0f, 5.0f, -2.0f)
		// vertice = ( 2.0f, 5.0f,  0.0f)
	}

	// Cap center vertex.
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	uint32 centerIndex = (uint32)meshData.Vertices.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i + 1);
		meshData.Indices32.push_back(baseIndex + i);
	}

	// indices = { 25, 21, 20 }
	// indices = { 25, 22, 21 }
	// indices = { 25, 23, 22 }
	// indices = { 25, 24, 23 }
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
	// 
	// Build bottom cap.
	//

	uint32 baseIndex = (uint32)meshData.Vertices.size();

	float y = -0.5f * height;

	// y = -0.5f * height
	// y = -0.5f *   10    = -5.0f;

	// Vertices of ring
	float dTheta = 2.0f * XM_PI / sliceCount;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		// PI equivale a metade da circunferencia, 180°, logo 2 * PI
		// equivale a 360°. 360° / sliceCount resulta no comprimento
		// da circunferencia de cada "pedaço" (slice). 360° / 4 = 90° = 1.5708 radianos
		//
		// dTheta = 2.0f  *     XM_PI     /  sliceCount
		// dTheta = 2.0f  *  3.141592741  /      4        = 1.5707963705

		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// x = bottomRadius  *  cos( i  *     dTheta    )
		// x =      5        *  cos( 0  *  1.5707963705 )  = 5 * cos(0)            =  5
		// x =      5        *  cos( 1  *  1.5707963705 )  = 5 * cos(1.5707963705) =  0
		// x =      5        *  cos( 2  *  1.5707963705 )  = 5 * cos(3.1415927410) = -5
		// x =      5        *  cos( 3  *  1.5707963705 )  = 5 * cos(4.7123891115) =  0
		// x =      5        *  cos( 4  *  1.5707963705 )  = 5 * cos(6.283185482 ) =  5

		// z = bottomRadius  *  sin( i  *     dTheta    )
		// z =      5        *  sin( 0  *  1.5707963705 )  = 5 * sin(0)            =  0
		// z =      5        *  sin( 1  *  1.5707963705 )  = 5 * sin(1.5707963705) =  5
		// z =      5        *  sin( 2  *  1.5707963705 )  = 5 * sin(3.1415927410) =  0
		// z =      5        *  sin( 3  *  1.5707963705 )  = 5 * sin(4.7123891115) = -5
		// z =      5        *  sin( 4  *  1.5707963705 )  = 5 * sin(6.283185482 ) =  0

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));

		// vertice = ( 5.0f, -5.0f,  0.0f)
		// vertice = ( 0.0f, -5.0f,  5.0f)
		// vertice = (-5.0f, -5.0f,  0.0f)
		// vertice = ( 0.0f, -5.0f, -5.0f)
		// vertice = ( 5.0f, -5.0f,  0.0f)
	}

	// Cap center vertex.
	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	uint32 centerIndex = (uint32)meshData.Vertices.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		meshData.Indices32.push_back(centerIndex);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}

	// indices = { 31, 26, 27 }
	// indices = { 31, 27, 28 }
	// indices = { 31, 28, 29 }
	// indices = { 31, 29, 30 }
}
