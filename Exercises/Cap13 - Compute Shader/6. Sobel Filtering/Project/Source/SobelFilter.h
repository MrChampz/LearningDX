#pragma once

#include <D3DUtil.h>
#include <GameTimer.h>

/* Performs a Sobel filtering operation on the topmost mip level of an input texture. */
class SobelFilter
{
public:
	// The width and height should match the dimensions of the input texture.
	// Recreate when the screen is resized.
	SobelFilter(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format);

	SobelFilter(const SobelFilter& rhs) = delete;
	SobelFilter& operator=(const SobelFilter& rhs) = delete;
	~SobelFilter() = default;

	void OnResize(UINT width, UINT height);

	void Execute(
		ID3D12GraphicsCommandList* cmdList,
		ID3D12RootSignature* rootSig,
		ID3D12PipelineState* pso,
		CD3DX12_GPU_DESCRIPTOR_HANDLE input);

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
		UINT descriptorSize);

	UINT GetDescriptorCount() const;

	CD3DX12_GPU_DESCRIPTOR_HANDLE GetOutputSrv();

private:
	void BuildDescriptors();
	void BuildResource();

private:
	// D3D Device
	ID3D12Device* m_d3dDevice = nullptr;

	// Input/Output resource related params.
	UINT m_Width = 0;
	UINT m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Handles to the resources.
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuUav;

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuUav;

	// Resource.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Output = nullptr;
};

