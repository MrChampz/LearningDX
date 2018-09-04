//***************************************************************************************
// Performs the calculations for the wave simulation. After the simulation has been
// updated, the client must copy the current solution into vertex buffers for rendering.
// This class only does the calculations, it does not do any drawing.
//***************************************************************************************
#ifndef GPU_WAVES_H
#define GPU_WAVES_H

#include <D3DUtil.h>
#include <GameTimer.h>

class GpuWaves
{
public:
	// Note that (m, n) should be divisible by 16 so there is no
	// remainder when we divide into thread groups.
	GpuWaves(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, int m, int n, float dx, float dt, float speed, float damping);
	GpuWaves(const GpuWaves& rhs) = delete;
	GpuWaves& operator=(const GpuWaves& rhs) = delete;
	~GpuWaves() = default;

	void Update(
		const GameTimer& timer,
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* pso);

	void Disturb(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* pso,
		UINT i, UINT j,
		float magnitude);

	void BuildResources(ID3D12GraphicsCommandList* cmdList);
	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize);

public:
	float GetWidth() const;
	float GetDepth() const;
	float GetSpatialStep() const;
	int GetRowCount() const;
	int GetColumnCount() const;
	int GetVertexCount() const;
	int GetTriangleCount() const;
	UINT GetDescriptorCount() const;

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetDisplacementMap() const;

private:
	int m_NumRows = 0;
	int m_NumCols = 0;

	int m_VertexCount = 0;
	int m_TriangleCount = 0;

	// Simulation constants we can precompute.
	float m_K[3];

	float m_TimeStep = 0.0f;
	float m_SpatialStep = 0.0f;

	ID3D12Device* m_d3dDevice = nullptr;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_PrevSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrSolSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_NextSolSrv;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_PrevSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrSolUav;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_NextSolUav;

	// Two for ping-ponging the textures.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_PrevSol = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_CurrSol = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_NextSol = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_PrevUploadBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_CurrUploadBuffer = nullptr;
};

#endif