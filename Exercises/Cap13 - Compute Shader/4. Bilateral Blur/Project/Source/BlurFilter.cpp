#include "BlurFilter.h"

BlurFilter::BlurFilter(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
	m_d3dDevice = device;

	m_Width = width;
	m_Height = height;
	m_Format = format;

	m_SigmaS = 2.5f;
	m_SigmaR = 0.01f;

	BuildResources();
}

void BlurFilter::OnResize(UINT width, UINT height)
{
	if ((m_Width != width) || (m_Height != height))
	{
		m_Width = width;
		m_Height = height;

		BuildResources();

		// New resource, so we need new descriptors to that resource.
		BuildDescriptors();
	}
}

void BlurFilter::OnKeyboardInput(const GameTimer& timer)
{
	if (GetAsyncKeyState(VK_HOME) & 0x8000)
		m_SigmaS += 0.1f;

	if (GetAsyncKeyState(VK_END) & 0x8000)
		m_SigmaS -= 0.1f;

	if (GetAsyncKeyState(VK_PRIOR) & 0x8000)
		m_SigmaR += 0.005f;

	if (GetAsyncKeyState(VK_NEXT) & 0x8000)
		m_SigmaR -= 0.005f;

	if (m_SigmaS < 0.001f)
		m_SigmaS = 0.001f;
	else if (m_SigmaS > 2.5f)
		m_SigmaS = 2.5f;

	if (m_SigmaR < 0.001f)
		m_SigmaR = 0.001f;
	else if (m_SigmaR > 80.0f)
		m_SigmaR = 80.0f;
}

void BlurFilter::Execute(
	ID3D12GraphicsCommandList* cmdList,
	ID3D12RootSignature* rootSig,
	ID3D12PipelineState* horzBlurPSO,
	ID3D12PipelineState* vertBlurPSO,
	ID3D12Resource* input,
	int blurCount)
{
	auto weights = CalcGaussSpatialWeights(m_SigmaS);
	int blurRadius = (int)weights.size() / 2;

	cmdList->SetComputeRootSignature(rootSig);

	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, 1, &m_SigmaR, 1);
	cmdList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 2);

	cmdList->ResourceBarrier(1,&CD3DX12_RESOURCE_BARRIER::Transition(
		input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap0.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	// Copy the input (back-buffer in this example) to BlurMap0.
	cmdList->CopyResource(m_BlurMap0.Get(), input);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_BlurMap1.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	for (int i = 0; i < blurCount; ++i)
	{
		//
		//	Horizontal blur pass.
		//

		cmdList->SetPipelineState(horzBlurPSO);

		cmdList->SetComputeRootDescriptorTable(1, m_Blur0GpuSrv);
		cmdList->SetComputeRootDescriptorTable(2, m_Blur1GpuUav);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the Compute Shader).
		UINT numGroupsX = (UINT)ceilf(m_Width / 256.0f);
		cmdList->Dispatch(numGroupsX, m_Height, 1);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap1.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

		//
		// Vertical blur pass.
		//

		cmdList->SetPipelineState(vertBlurPSO);

		cmdList->SetComputeRootDescriptorTable(1, m_Blur1GpuSrv);
		cmdList->SetComputeRootDescriptorTable(2, m_Blur0GpuUav);

		// How many groups do we need to dispatch to cover a column of pixels, where each
		// group covers 256 pixels (the 256 is defined in the Compute Shader).
		UINT numGroupsY = (UINT)ceilf(m_Height / 256.0f);
		cmdList->Dispatch(m_Width, numGroupsY, 1);

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap0.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_BlurMap1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	}
}

void BlurFilter::BuildDescriptors(
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
	UINT descriptorSize)
{
	// Save references to the descriptors.
	m_Blur0CpuSrv = hCpuDescriptor;
	m_Blur0CpuUav = hCpuDescriptor.Offset(1, descriptorSize);
	m_Blur1CpuSrv = hCpuDescriptor.Offset(1, descriptorSize);
	m_Blur1CpuUav = hCpuDescriptor.Offset(1, descriptorSize);

	m_Blur0GpuSrv = hGpuDescriptor;
	m_Blur0GpuUav = hGpuDescriptor.Offset(1, descriptorSize);
	m_Blur1GpuSrv = hGpuDescriptor.Offset(1, descriptorSize);
	m_Blur1GpuUav = hGpuDescriptor.Offset(1, descriptorSize);

	BuildDescriptors();
}

ID3D12Resource* BlurFilter::GetOutput()
{
	return m_BlurMap0.Get();
}

std::vector<float> BlurFilter::CalcGaussSpatialWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 6.
	int blurRadius = (int)ceil(2.0f * sigma);

	assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;
	
	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	return weights;
}

void BlurFilter::BuildDescriptors()
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

	m_d3dDevice->CreateShaderResourceView(m_BlurMap0.Get(), &srvDesc, m_Blur0CpuSrv);
	m_d3dDevice->CreateUnorderedAccessView(m_BlurMap0.Get(), nullptr, &uavDesc, m_Blur0CpuUav);

	m_d3dDevice->CreateShaderResourceView(m_BlurMap1.Get(), &srvDesc, m_Blur1CpuSrv);
	m_d3dDevice->CreateUnorderedAccessView(m_BlurMap1.Get(), nullptr, &uavDesc, m_Blur1CpuUav);
}

void BlurFilter::BuildResources()
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
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_BlurMap0)));

	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_BlurMap1)));
}