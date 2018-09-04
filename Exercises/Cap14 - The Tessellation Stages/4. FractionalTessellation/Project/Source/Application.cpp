#include "Application.h"

Application::Application(HINSTANCE hInst)
	: D3DApp(hInst)
{

}

Application::~Application()
{

}

bool Application::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	BuildQuadPatchGeometry();
	BuildRenderItems();
	BuildFrameResources();

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildPSOs();

	// Execute the initialization commands.
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void Application::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, proj);
}

void Application::Update(const GameTimer& timer)
{
	OnKeyboardInput(timer);
	UpdateCamera(timer);

	// Cycle through the circular frame resource array.
	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % g_NumFrameResources;
	m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (m_CurrFrameResource->Fence != 0 && m_Fence->GetCompletedValue() < m_CurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjectCBs(timer);
	UpdateMainPassCB(timer);
}

void Application::Draw(const GameTimer& timer)
{
	auto cmdListAlloc = m_CurrFrameResource->CmdListAlloc;
	// Reuse the memory associated with command recording.
	// We can only reset when the command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandLists.
	// Reusing the command list reuses memory.
	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["Tessellation"].Get()));

	// Set the viewport and scissor rect. This needs to be reset whenever the command list is reset.
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	m_CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), Colors::Black, 0, nullptr);
	m_CommandList->ClearDepthStencilView(GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(1, &GetCurrentBackBufferView(), true, &GetDepthStencilView());

	// Bind the root signature.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// Bind per-pass constant buffer. We only need to this once per-pass.
	auto passCB = m_CurrFrameResource->PassCB->GetResource();
	m_CommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	// Draw opaque render items.
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Opaque]);

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	// Swap the back and front buffers.
	ThrowIfFailed(m_SwapChain->Present(1, 0));
	m_CurrentBackBuffer = (m_CurrentBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	m_CurrFrameResource->Fence = ++m_CurrentFence;

	// Add an instruction to the command queue to set a new fence point.
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence);
}

void Application::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = m_InitialMousePos.x = x;
	m_LastMousePos.y = m_InitialMousePos.y = y;

	SetCapture(m_AppWnd);
	ShowCursor(false);
}

void Application::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
	ClientToScreen(m_AppWnd, &m_InitialMousePos);
	SetCursorPos(m_InitialMousePos.x, m_InitialMousePos.y);
	ShowCursor(true);
}

void Application::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		// Update angles based on input to orbit camera around box.
		m_Theta += dx;
		m_Phi += dy;

		// Restrict the angle m_Phi.
		m_Phi = MathHelper::Clamp(m_Phi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.05f * static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.05f * static_cast<float>(y - m_LastMousePos.y);

		// Update the camera radius based on input.
		m_Radius += dx - dy;

		// Restrict the radius.
		m_Radius = MathHelper::Clamp(m_Radius, 5.0f, 150.0f);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

void Application::OnKeyboardInput(const GameTimer& timer)
{
	
}

void Application::UpdateCamera(const GameTimer& timer)
{
	// Convert Spherical to Cartesian coordinates.
	m_EyePos.x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
	m_EyePos.z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
	m_EyePos.y = m_Radius * cosf(m_Phi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(m_EyePos.x, m_EyePos.y, m_EyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, view);
}

void Application::UpdateObjectCBs(const GameTimer& timer)
{
	auto currObjectCB = m_CurrFrameResource->ObjectCB.get();
	for (auto& item : m_AllRenderables)
	{
		// Only update the cbuffer data if the constants have changed.
		// This needs to be tracked per frame resource.
		if (item->NumFrameDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&item->World);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

			currObjectCB->CopyData(item->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			item->NumFrameDirty--;
		}
	}
}

void Application::UpdateMainPassCB(const GameTimer& timer)
{
	XMMATRIX view = XMLoadFloat4x4(&m_View);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	m_MainPassCB.EyePosW = m_EyePos;
	m_MainPassCB.RenderTargetSize = XMFLOAT2((float)m_ClientWidth, (float)m_ClientHeight);
	m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_ClientWidth, 1.0f / m_ClientHeight);
	m_MainPassCB.NearZ = 1.0f;
	m_MainPassCB.FarZ = 1000.0f;
	m_MainPassCB.TotalTime = timer.TotalTime();
	m_MainPassCB.DeltaTime = timer.DeltaTime();

	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void Application::BuildQuadPatchGeometry()
{
	std::array<XMFLOAT3, 4> vertices =
	{
		XMFLOAT3(-10.0f, 0.0f,  10.0f),
		XMFLOAT3( 10.0f, 0.0f,  10.0f),
		XMFLOAT3(-10.0f, 0.0f, -10.0f),
		XMFLOAT3( 10.0f, 0.0f, -10.0f)
	};

	std::array<std::int16_t, 4> indices = { 0, 1, 2, 3 };

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(XMFLOAT3);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "quadGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(XMFLOAT3);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubMeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = indices.size();
	quadSubmesh.StartIndexLocation = 0;
	quadSubmesh.BaseVertexLocation = 0;
	geo->DrawArgs["quad"] = quadSubmesh;

	m_Geometries[geo->Name] = std::move(geo);
}

void Application::BuildRenderItems()
{
	auto quadPatchItem = std::make_unique<Renderable>();
	quadPatchItem->ObjCBIndex = 0;
	quadPatchItem->Geo = m_Geometries["quadGeo"].get();
	quadPatchItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
	quadPatchItem->IndexCount = quadPatchItem->Geo->DrawArgs["quad"].IndexCount;
	quadPatchItem->StartIndexLocation = quadPatchItem->Geo->DrawArgs["quad"].StartIndexLocation;
	quadPatchItem->BaseVertexLocation = quadPatchItem->Geo->DrawArgs["quad"].BaseVertexLocation;
	m_RenderableLayer[(int)RenderLayer::Opaque].push_back(quadPatchItem.get());

	m_AllRenderables.push_back(std::move(quadPatchItem));
}

void Application::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Renderable*>& renderables)
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	auto objCB = m_CurrFrameResource->ObjectCB->GetResource();

	// For each render item..
	for (size_t i = 0; i < renderables.size(); ++i)
	{
		auto item = renderables[i];

		cmdList->IASetVertexBuffers(0, 1, &item->Geo->GetVertexBufferView());
		cmdList->IASetIndexBuffer(&item->Geo->GetIndexBufferView());
		cmdList->IASetPrimitiveTopology(item->PrimitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCB->GetGPUVirtualAddress() + item->ObjCBIndex * objCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

		cmdList->DrawIndexedInstanced(item->IndexCount, 1, item->StartIndexLocation, item->BaseVertexLocation, 0);
	}
}

void Application::BuildFrameResources()
{
	for (int i = 0; i < g_NumFrameResources; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(
			m_d3dDevice.Get(), 1, (UINT)m_AllRenderables.size()));
	}
}

void Application::BuildRootSignature()
{
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	// Create root CBVs.
	// Performance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// Create a root signature with a single slot which points
	// to a descriptor range consisting of a single constant buffer.
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	
	HRESULT result = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(result);

	ThrowIfFailed(m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
}

void Application::BuildShadersAndInputLayout()                       
{
	m_Shaders["TessellationVS"] = D3DUtil::LoadBinary(L"Shaders/Tessellation_VS.cso");
	m_Shaders["TessellationHS"] = D3DUtil::LoadBinary(L"Shaders/Tessellation_HS.cso");
	m_Shaders["TessellationDS"] = D3DUtil::LoadBinary(L"Shaders/Tessellation_DS.cso");
	m_Shaders["TessellationPS"] = D3DUtil::LoadBinary(L"Shaders/Tessellation_PS.cso");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void Application::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tessellationPsoDesc;
	ZeroMemory(&tessellationPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	//
	// PSO for tessellating objects.
	//

	tessellationPsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	tessellationPsoDesc.pRootSignature = m_RootSignature.Get();
	tessellationPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["TessellationVS"]->GetBufferPointer()),
		m_Shaders["TessellationVS"]->GetBufferSize()
	};
	tessellationPsoDesc.HS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["TessellationHS"]->GetBufferPointer()),
		m_Shaders["TessellationHS"]->GetBufferSize()
	};
	tessellationPsoDesc.DS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["TessellationDS"]->GetBufferPointer()),
		m_Shaders["TessellationDS"]->GetBufferSize()
	};
	tessellationPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["TessellationPS"]->GetBufferPointer()),
		m_Shaders["TessellationPS"]->GetBufferSize()
	};
	tessellationPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tessellationPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	tessellationPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tessellationPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tessellationPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tessellationPsoDesc.SampleMask = UINT_MAX;
	tessellationPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	tessellationPsoDesc.NumRenderTargets = 1;
	tessellationPsoDesc.RTVFormats[0] = m_BackBufferFormat;
	tessellationPsoDesc.DSVFormat = m_DepthStencilFormat;
	tessellationPsoDesc.SampleDesc.Count = m_msaaState ? m_msaaLevel : 1;
	tessellationPsoDesc.SampleDesc.Quality = m_msaaState ? (m_msaaQuality - 1) : 0;
	
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&tessellationPsoDesc, IID_PPV_ARGS(&m_PSOs["Tessellation"])));
}