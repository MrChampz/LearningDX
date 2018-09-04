#include "SobelFilter.h"

SobelFilter::SobelFilter(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	m_d3dDevice = device;

	m_Width = width;
	m_Height = height;
	m_Format = format;

	BuildResource();
}

void SobelFilter::OnResize(UINT width, UINT height)
{
	if ((m_Width != width) || (m_Height != height))
	{
		m_Width = width;
		m_Height = height;

		BuildResource();

		// New resource, so we need new descriptors to that resource.
		BuildDescriptors();
	}
}

void SobelFilter::Execute(
	ID3D12GraphicsCommandList* cmdList,
	ID3D12RootSignature* rootSig,
	ID3D12PipelineState* pso,
	CD3DX12_GPU_DESCRIPTOR_HANDLE input)
{
	cmdList->SetComputeRootSignature(rootSig);
	cmdList->SetPipelineState(pso);

	cmdList->SetComputeRootDescriptorTable(0, input);
	cmdList->SetComputeRootDescriptorTable(2, m_GpuUav);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_Output.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	// How many groups do we need to dispatch to cover a quad of pixels, where each
	// group covers 16x16 pixels (the 16 is defined in the Compute Shader).
	UINT numGroupsX = (UINT)ceilf(m_Width  / 16.0f);
	UINT numGroupsY = (UINT)ceilf(m_Height / 16.0f);
	cmdList->Dispatch(numGroupsX, numGroupsY, 1);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_Output.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void SobelFilter::BuildDescriptors(
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
	UINT descriptorSize)
{
	// Save references to the descriptors.
	m_CpuSrv = hCpuDescriptor;
	m_CpuUav = hCpuDescriptor.Offset(1, descriptorSize);
	
	m_GpuSrv = hGpuDescriptor;
	m_GpuUav = hGpuDescriptor.Offset(1, descriptorSize);

	BuildDescriptors();
}

UINT SobelFilter::GetDescriptorCount() const
{
	return 2;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE SobelFilter::GetOutputSrv()
{
	return m_GpuSrv;
}

void SobelFilter::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = m_Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	m_d3dDevice->CreateShaderResourceView(m_Output.Get(), &srvDesc, m_CpuSrv);
	m_d3dDevice->CreateUnorderedAccessView(m_Output.Get(), nullptr, &uavDesc, m_CpuUav);
}

void SobelFilter::BuildResource()
{
	// Note, compressed formats cannot be used for UAV. We get error like:
	// ERROR: ID3D12Device::CreateTexture2D: The format (0x4d, BC3_UNORM)
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView. Therefore this format
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_Output)));
}