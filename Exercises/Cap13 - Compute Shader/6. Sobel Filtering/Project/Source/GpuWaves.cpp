#include "GpuWaves.h"

#include <algorithm>
#include <vector>
#include <cassert>

GpuWaves::GpuWaves(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
	int m, int n, float dx, float dt, float speed, float damping)
{
	m_d3dDevice = device;

	m_NumRows = m;
	m_NumCols = n;

	assert((m * n) % 256 == 0);

	m_VertexCount = m * n;
	m_TriangleCount = (m - 1) * (n - 1) * 2;

	m_TimeStep = dt;
	m_SpatialStep = dx;

	float d = damping * dt + 2.0f;
	float e = (speed * speed) * (dt * dt) / (dx * dx);
	m_K[0] = (damping * dt - 2.0f) / d;
	m_K[1] = (4.0f - 8.0f * e) / d;
	m_K[2] = (2.0f * e) / d;

	BuildResources(cmdList);
}

void GpuWaves::Update(
	const GameTimer& timer,
	ID3D12GraphicsCommandList* cmdList,
	ID3D12RootSignature* rootSig,
	ID3D12PipelineState* pso)
{
	static float t = 0.0f;

	// Accumulate time.
	t += timer.DeltaTime();

	cmdList->SetPipelineState(pso);
	cmdList->SetComputeRootSignature(rootSig);

	// Only update the simulation at the specified time step.
	if (t >= m_TimeStep)
	{
		// Set the update constants.
		cmdList->SetComputeRoot32BitConstants(0, 3, m_K, 0);

		cmdList->SetComputeRootDescriptorTable(1, m_PrevSolUav);
		cmdList->SetComputeRootDescriptorTable(2, m_CurrSolUav);
		cmdList->SetComputeRootDescriptorTable(3, m_NextSolUav);

		// How many groups do we need to dispatch to cover the wave grid.
		// Note that m_NumRows and m_NumCols should be divisible by 16
		// so there is no remainder.
		UINT numGroupsX = m_NumCols / 16;
		UINT numGroupsY = m_NumRows / 16;
		cmdList->Dispatch(numGroupsX, numGroupsY, 1);

		//
		// Ping-pong buffers in preparation for the next update.
		// The previous solution is no longer needed and becomes the target of the next solution in the next update.
		// The current solution becomes the previous solution.
		// The next solution becomes the current solution.
		//

		auto tempRes = m_PrevSol;
		m_PrevSol    = m_CurrSol;
		m_CurrSol    = m_NextSol;
		m_NextSol    = tempRes;

		auto tempSrv = m_PrevSolSrv;
		m_PrevSolSrv = m_CurrSolSrv;
		m_CurrSolSrv = m_NextSolSrv;
		m_NextSolSrv = tempSrv;

		auto tempUav = m_PrevSolUav;
		m_PrevSolUav = m_CurrSolUav;
		m_CurrSolUav = m_NextSolUav;
		m_NextSolUav = tempUav;

		// Reset time
		t = 0.0f;

		// The current solution needs to be able to be read by the vertex shader, so change its state to GENERIC_READ.
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_CurrSol.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
	}
}

void GpuWaves::Disturb(
	ID3D12GraphicsCommandList* cmdList,
	ID3D12RootSignature* rootSig,
	ID3D12PipelineState* pso,
	UINT i, UINT j,
	float magnitude)
{
	cmdList->SetPipelineState(pso);
	cmdList->SetComputeRootSignature(rootSig);

	// Set the disturb constants.
	UINT disturbIndex[2] = { j, i };
	cmdList->SetComputeRoot32BitConstants(0, 1, &magnitude, 3);
	cmdList->SetComputeRoot32BitConstants(0, 2, disturbIndex, 4);

	cmdList->SetComputeRootDescriptorTable(3, m_CurrSolUav);

	// The current solution is in the GENERIC_READ state so it can be read by the vertex shader.
	// Change it to UNORDERED_ACCESS for the compute shader. Note that a UAV can still be
	// read in a compute shader.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_CurrSol.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	// One thread group kicks off one thread, which displaces the height of one
	// vertex and its neighbors.
	cmdList->Dispatch(1, 1, 1);
}

void GpuWaves::BuildResources(ID3D12GraphicsCommandList* cmdList)
{
	// All the textures for the wave simulation will be bound as a shader resource and
	// unordered access view at some point since we ping-pong the buffers.

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_NumCols;
	texDesc.Height = m_NumRows;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_PrevSol)));

	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_CurrSol)));

	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_NextSol)));

	m_PrevSol->SetName(L"PrevSol");
	m_CurrSol->SetName(L"CurrSol");
	m_NextSol->SetName(L"NextSol");

	//
	// In order to copy CPU memory data into out default buffer, we need to create
	// an intermediate upload heap.
	//

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_CurrSol.Get(), 0, num2DSubresources);

	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_PrevUploadBuffer.GetAddressOf())));

	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_CurrUploadBuffer.GetAddressOf())));

	m_PrevUploadBuffer->SetName(L"PrevUploadBuffer");
	m_CurrUploadBuffer->SetName(L"CurrUploadBuffer");

	// Describe the data we want to copy into the default buffer.
	std::vector<float> data(m_NumRows * m_NumCols, 0.0f);
	for (int i = 0; i < data.size(); ++i)
		data[i] = 0.0f;

	D3D12_SUBRESOURCE_DATA subresData = {};
	subresData.pData = data.data();
	subresData.RowPitch = m_NumCols * sizeof(float);
	subresData.SlicePitch = subresData.RowPitch * m_NumRows;

	//
	// Schedule to copy the data to the default resource, and change states.
	// Note that m_CurrSol is put in the GENERIC_READ state so it can be
	// read by a shader.
	//

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_PrevSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(cmdList, m_PrevSol.Get(), m_PrevUploadBuffer.Get(), 0, 0, num2DSubresources, &subresData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_PrevSol.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_CurrSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(cmdList, m_CurrSol.Get(), m_CurrUploadBuffer.Get(), 0, 0, num2DSubresources, &subresData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_CurrSol.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_NextSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
}

void GpuWaves::BuildDescriptors(
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
	UINT descriptorSize)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	m_d3dDevice->CreateShaderResourceView(m_PrevSol.Get(), &srvDesc, hCpuDescriptor);
	m_d3dDevice->CreateShaderResourceView(m_CurrSol.Get(), &srvDesc, hCpuDescriptor.Offset(1, descriptorSize));
	m_d3dDevice->CreateShaderResourceView(m_NextSol.Get(), &srvDesc, hCpuDescriptor.Offset(1, descriptorSize));

	m_d3dDevice->CreateUnorderedAccessView(m_PrevSol.Get(), nullptr, &uavDesc, hCpuDescriptor.Offset(1, descriptorSize));
	m_d3dDevice->CreateUnorderedAccessView(m_CurrSol.Get(), nullptr, &uavDesc, hCpuDescriptor.Offset(1, descriptorSize));
	m_d3dDevice->CreateUnorderedAccessView(m_NextSol.Get(), nullptr, &uavDesc, hCpuDescriptor.Offset(1, descriptorSize));

	// Save references to the GPU descriptors.
	m_PrevSolSrv = hGpuDescriptor;
	m_CurrSolSrv = hGpuDescriptor.Offset(1, descriptorSize);
	m_NextSolSrv = hGpuDescriptor.Offset(1, descriptorSize);
	m_PrevSolUav = hGpuDescriptor.Offset(1, descriptorSize);
	m_CurrSolUav = hGpuDescriptor.Offset(1, descriptorSize);
	m_NextSolUav = hGpuDescriptor.Offset(1, descriptorSize);
}

float GpuWaves::GetWidth() const
{
	return m_NumCols * m_SpatialStep;
}

float GpuWaves::GetDepth() const
{
	return m_NumRows * m_SpatialStep;
}

float GpuWaves::GetSpatialStep() const
{
	return m_SpatialStep;
}

int GpuWaves::GetRowCount() const
{
	return m_NumRows;
}

int GpuWaves::GetColumnCount() const
{
	return m_NumCols;
}

int GpuWaves::GetVertexCount() const
{
	return m_VertexCount;
}

int GpuWaves::GetTriangleCount() const
{
	return m_TriangleCount;
}

UINT GpuWaves::GetDescriptorCount() const
{
	// Number of descriptors in heap to reserve for GpuWaves.
	return 6;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE GpuWaves::GetDisplacementMap() const
{
	return m_CurrSolSrv;
}
