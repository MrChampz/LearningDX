#pragma once

#include <D3DUtil.h>
#include <GameTimer.h>

/* Performs a blur operation on the topmost mip level of an input texture. */
class BlurFilter
{
public:
	// The width and height should match the dimensions of the input texture to blur.
	// Recreate when the screen is resized.
	BlurFilter(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format);

	BlurFilter(const BlurFilter& rhs) = delete;
	BlurFilter& operator=(const BlurFilter& rhs) = delete;
	~BlurFilter() = default;

	void OnResize(UINT width, UINT height);
	
	void OnKeyboardInput(const GameTimer& timer);

	// Blurs the input texture blurCount times.
	void Execute(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* horzBlurPSO,
		ID3D12PipelineState* vertBlurPSO,
		ID3D12Resource* input,
		int blurCount);

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize);

	ID3D12Resource* GetOutput();

private:
	std::vector<float> CalcGaussSpatialWeights(float sigma);

	void BuildDescriptors();
	void BuildResources();

private:
	const int MaxBlurRadius = 5;

	float m_SigmaS; // The spatial sigma.
	float m_SigmaR; // The range sigma.

	// D3D Device
	ID3D12Device* m_d3dDevice = nullptr;

	// Input/Output resource related params.
	UINT m_Width = 0;
	UINT m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Handles to the resources.
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur0CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur0CpuUav;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur1CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_Blur1CpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur0GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur0GpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur1GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_Blur1GpuUav;

	// Blurred resources.
	// Two for ping-ponging the textures.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_BlurMap0 = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_BlurMap1 = nullptr;
};

