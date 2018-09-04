#pragma once

#include <D3DUtil.h>

class RenderTarget
{
public:
	// The width and height should match the dimensions of the input texture.
	// Recreate when the screen is resized.
	RenderTarget(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format);

	RenderTarget(const RenderTarget& rhs) = delete;
	RenderTarget& operator=(const RenderTarget& rhs) = delete;
	~RenderTarget() = default;

	void OnResize(UINT width, UINT height);

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv);

	ID3D12Resource* GetResource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrv();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv();

private:
	void BuildDescriptors();
	void BuildResource();

private:
	// D3D Device
	ID3D12Device * m_d3dDevice = nullptr;

	// Input/Output resource related params.
	UINT m_Width = 0;
	UINT m_Height = 0;
	DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Handles to the resources.
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuRtv;

	// Resource.
	Microsoft::WRL::ComPtr<ID3D12Resource> m_OffscreenTex = nullptr;
};