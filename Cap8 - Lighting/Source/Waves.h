//***************************************************************************************
// Waves.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs the calculations for the wave simulation.After the simulation has been
// updated, the client must copy the current solution into vertex buffers for rendering.
// This class only does the calculations, it does not do any drawing.
//***************************************************************************************
#ifndef WAVES_H
#define WAVES_H

#include <vector>
#include <DirectXMath.h>

class Waves
{
public:
	Waves(int m, int n, float dx, float dt, float speed, float damping);
	Waves(const Waves& rhs) = delete;
	Waves& operator=(const Waves& rhs) = delete;
	~Waves();

	void Update(float dt);
	void Disturb(int i, int j, float magnitude);

public:
	float GetWidth() const;
	float GetDepth() const;
	int GetRowCount() const;
	int GetColumnCount() const;
	int GetVertexCount() const;
	int GetTriangleCount() const;

	// Returns the solution at the ith grid point.
	const DirectX::XMFLOAT3& GetPosition(int i) { return m_CurrSolution[i]; }

	// Returns the solution normal at the ith grid point.
	const DirectX::XMFLOAT3& GetNormal(int i) { return m_Normals[i]; }

	// Returns the unit tangent vector at the ith grid point in the local x-axis direction.
	const DirectX::XMFLOAT3& GetTangentX(int i) { return m_TangentX[i]; }

private:
	int m_NumRows = 0;
	int m_NumCols = 0;

	int m_VertexCount = 0;
	int m_TriangleCount = 0;

	// Simulation constants we can precompute.
	float m_K1 = 0.0f;
	float m_K2 = 0.0f;
	float m_K3 = 0.0f;

	float m_TimeStep = 0.0f;
	float m_SpatialStep = 0.0f;

	std::vector<DirectX::XMFLOAT3> m_PrevSolution;
	std::vector<DirectX::XMFLOAT3> m_CurrSolution;
	std::vector<DirectX::XMFLOAT3> m_Normals;
	std::vector<DirectX::XMFLOAT3> m_TangentX;
};

#endif // WAVES_H