#include "CylinderApp.h"

CylinderApp::CylinderApp(HINSTANCE hInst)
	: D3DApp(hInst)
{

}

CylinderApp::~CylinderApp()
{

}

bool CylinderApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	m_Waves = std::make_unique<Waves>(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);

	LoadTextures();
	BuildBoxGeometry();
	BuildLandGeometry();
	BuildWavesGeometryBuffers();
	BuildCircleGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();

	BuildDescriptorHeaps();
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

void CylinderApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, proj);
}

void CylinderApp::Update(const GameTimer& timer)
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

	AnimateMaterials(timer);
	UpdateObjectCBs(timer);
	UpdateMaterialCBs(timer);
	UpdateMainPassCB(timer);
	UpdateWaves(timer);
}

void CylinderApp::Draw(const GameTimer& timer)
{
	auto cmdListAlloc = m_CurrFrameResource->CmdListAlloc;
	// Reuse the memory associated with command recording.
	// We can only reset when the command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandLists.
	// Reusing the command list reuses memory.
	if (m_IsWireframe)
	{
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["Opaque_Wireframe"].Get()));
	}
	else
	{
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSOs["Opaque"].Get()));
	}

	// Set the viewport and scissor rect. This needs to be reset whenever the command list is reset.
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	m_CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), (float*)&m_MainPassCB.FogColor, 0, nullptr);
	m_CommandList->ClearDepthStencilView(GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(1, &GetCurrentBackBufferView(), true, &GetDepthStencilView());

	// Bind the descriptor heaps.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvDescriptorHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// Bind the root signature.
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// Bind per-pass constant buffer. We only need to this once per-pass.
	auto passCB = m_CurrFrameResource->PassCB->GetResource();
	m_CommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

	// Draw opaque render items.
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Opaque]);

	if (m_IsWireframe)
	{
		// Draw alpha tested and transparent render items as wireframed.
		DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::AlphaTested]);
		DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Cylinder]);
		DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Transparent]);
	}
	else
	{
		// Draw alpha tested render items.
		m_CommandList->SetPipelineState(m_PSOs["AlphaTested"].Get());
		DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::AlphaTested]);

		// Draw cylinder generator render items.
		m_CommandList->SetPipelineState(m_PSOs["Cylinder"].Get());
		DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Cylinder]);

		// Draw transparent render items.
		m_CommandList->SetPipelineState(m_PSOs["Transparent"].Get());
		DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Transparent]);
	}

	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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

void CylinderApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = m_InitialMousePos.x = x;
	m_LastMousePos.y = m_InitialMousePos.y = y;

	SetCapture(m_AppWnd);
	ShowCursor(false);
}

void CylinderApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
	ClientToScreen(m_AppWnd, &m_InitialMousePos);
	SetCursorPos(m_InitialMousePos.x, m_InitialMousePos.y);
	ShowCursor(true);
}

void CylinderApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void CylinderApp::OnKeyboardInput(const GameTimer& timer)
{
	if (GetAsyncKeyState('1') & 0x8000)
		m_IsWireframe = true;
	else
		m_IsWireframe = false;
}

void CylinderApp::AnimateMaterials(const GameTimer& timer)
{
	// Scroll the water material texture coordinates.
	auto waterMat = m_Materials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * timer.DeltaTime();
	tv += 0.02f * timer.DeltaTime();

	if (tu >= 1.0f)
	{
		tu -= 1.0f;
	}

	if (tv >= 1.0f)
	{
		tv -= 1.0f;
	}

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = g_NumFrameResources;
}

void CylinderApp::UpdateCamera(const GameTimer& timer)
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

void CylinderApp::UpdateObjectCBs(const GameTimer& timer)
{
	auto currObjectCB = m_CurrFrameResource->ObjectCB.get();
	for (auto& item : m_AllRenderables)
	{
		// Only update the cbuffer data if the constants have changed.
		// This needs to be tracked per frame resource.
		if (item->NumFrameDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&item->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&item->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(item->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			item->NumFrameDirty--;
		}
	}
}

void CylinderApp::UpdateMaterialCBs(const GameTimer& timer)
{
	auto currMaterialCB = m_CurrFrameResource->MaterialCB.get();
	for (auto& e : m_Materials)
	{
		// Only update the cbuffer if the constants have changed. If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void CylinderApp::UpdateMainPassCB(const GameTimer& timer)
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

	m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	m_MainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.9f };
	m_MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[1].Strength = { 0.5f, 0.5f, 0.5f };
	m_MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void CylinderApp::UpdateWaves(const GameTimer& timer)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if ((timer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, m_Waves->GetRowCount() - 5);
		int j = MathHelper::Rand(4, m_Waves->GetColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		m_Waves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	m_Waves->Update(timer.DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = m_CurrFrameResource->WavesVB.get();
	for (int i = 0; i < m_Waves->GetVertexCount(); ++i)
	{
		Vertex v;

		v.Pos = m_Waves->GetPosition(i);
		v.Normal = m_Waves->GetNormal(i);

		// Derive tex-coords from position by 
		// mapping [-w / 2, w / 2] --> [0, 1]
		v.TexCoord.x = 0.5f + v.Pos.x / m_Waves->GetWidth();
		v.TexCoord.y = 0.5f - v.Pos.z / m_Waves->GetDepth();

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave renderable to the current frame VB.
	m_WavesRenderable->Geo->VertexBufferGPU = currWavesVB->GetResource();
}

float CylinderApp::GetHillsHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 CylinderApp::GetHillsNormal(float x, float z) const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n
	(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z)
	);

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void CylinderApp::LoadTextures()
{
	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->Filename = L"Textures/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), grassTex->Filename.c_str(),
		grassTex->Resource, grassTex->UploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->Name = "waterTex";
	waterTex->Filename = L"Textures/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), waterTex->Filename.c_str(),
		waterTex->Resource, waterTex->UploadHeap));

	auto fenceTex = std::make_unique<Texture>();
	fenceTex->Name = "fenceTex";
	fenceTex->Filename = L"Textures/WireFence.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), fenceTex->Filename.c_str(),
		fenceTex->Resource, fenceTex->UploadHeap));

	auto bricks = std::make_unique<Texture>();
	bricks->Name = "bricks";
	bricks->Filename = L"Textures/bricks.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), bricks->Filename.c_str(),
		bricks->Resource, bricks->UploadHeap));

	m_Textures[grassTex->Name] = std::move(grassTex);
	m_Textures[waterTex->Name] = std::move(waterTex);
	m_Textures[fenceTex->Name] = std::move(fenceTex);
	m_Textures[bricks->Name] = std::move(bricks);
}

void CylinderApp::BuildBoxGeometry()
{
	GeometryGenerator::MeshData box = GeometryGenerator::CreateBox(8.0f, 8.0f, 8.0f, 3);

	SubMeshGeometry boxSubMesh;
	boxSubMesh.IndexCount = (UINT)box.Indices32.size();
	boxSubMesh.StartIndexLocation = 0;
	boxSubMesh.BaseVertexLocation = 0;

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexCoord = box.Vertices[i].TexCoord;
	}

	std::vector<std::uint16_t> indices = box.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubMesh;

	m_Geometries[geo->Name] = std::move(geo);
}

void CylinderApp::BuildLandGeometry()
{
	GeometryGenerator::MeshData grid = GeometryGenerator::CreateGrid(160.0f, 160.0f, 50, 50);

	// Extract the vertex elements we are interested and apply the height function to
	// each vertex. In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		auto& p = grid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
		vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexCoord = grid.Vertices[i].TexCoord;
	}

	std::vector<std::uint16_t> indices = grid.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubMeshGeometry subMesh;
	subMesh.IndexCount = (UINT)indices.size();
	subMesh.StartIndexLocation = 0;
	subMesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = subMesh;

	m_Geometries["landGeo"] = std::move(geo);
}

void CylinderApp::BuildWavesGeometryBuffers()
{
	std::vector<std::uint16_t> indices(3 * m_Waves->GetTriangleCount()); // 3 indices per face
	assert(m_Waves->GetVertexCount() < 0x0000ffff);

	// Iterate over each quad.
	int m = m_Waves->GetRowCount();
	int n = m_Waves->GetColumnCount();

	int k = 0;
	for (int i = 0; i < m - 1; ++i)
	{
		for (int j = 0; j < n - 1; ++j)
		{
			indices[k + 0] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	UINT vbByteSize = m_Waves->GetVertexCount() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubMeshGeometry subMesh;
	subMesh.IndexCount = (UINT)indices.size();
	subMesh.StartIndexLocation = 0;
	subMesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = subMesh;

	m_Geometries["waterGeo"] = std::move(geo);
}

void CylinderApp::BuildCircleGeometry()
{
	// Circle offset in y-axis (local coordinate system).
	float y = 0.0f;

	// Circle radius.
	float radius = 5.0f;

	// Height of the generated cylinder.
	float height = 10.0f;

	// Circle slice count.
	const int sliceCount = 32;

	// Delta of theta angle.
	float dTheta = 2.0f * XM_PI / sliceCount;

	std::array<Vertex, sliceCount + 1> vertices;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		// Generate the vertex position based on the delta of theta.
		float x = cosf(i * dTheta);
		float z = sinf(i * dTheta);

		vertices[i].Pos = XMFLOAT3(radius * x, y, radius * z);

		XMFLOAT3 tangent = XMFLOAT3(-x, 0.0f, -z);
		XMFLOAT3 bitangent = XMFLOAT3(0.0f, -height, 0.0f);

		// Generate the normal vector.
		XMVECTOR T = XMLoadFloat3(&tangent);
		XMVECTOR B = XMLoadFloat3(&bitangent);
		XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
		XMStoreFloat3(&vertices[i].Normal, N);

		// Generate u-axis texture coordinate.
		// The v-axis will be defined in the geometry shader.
		vertices[i].TexCoord.x = (float)i / sliceCount;
		vertices[i].TexCoord.y = 1.0f;
	}

	std::array<std::uint16_t, sliceCount + 1> indices;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		indices[i] = i;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "circleGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = D3DUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubMeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["circle"] = submesh;

	m_Geometries[geo->Name] = std::move(geo);
}

void CylinderApp::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseSrvHeapIndex = 0;
	grass->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;

	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseSrvHeapIndex = 1;
	water->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	water->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	water->Roughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->Name = "wirefence";
	wirefence->MatCBIndex = 2;
	wirefence->DiffuseSrvHeapIndex = 2;
	wirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	wirefence->Roughness = 0.25f;

	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->MatCBIndex = 3;
	bricks->DiffuseSrvHeapIndex = 3;
	bricks->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	bricks->Roughness = 0.125f;

	m_Materials["grass"] = std::move(grass);
	m_Materials["water"] = std::move(water);
	m_Materials["wirefence"] = std::move(wirefence);
	m_Materials["bricks"] = std::move(bricks);
}

void CylinderApp::BuildRenderItems()
{
	auto wavesItem = std::make_unique<Renderable>();
	wavesItem->ObjCBIndex = 0;
	wavesItem->Mat = m_Materials["water"].get();
	wavesItem->Geo = m_Geometries["waterGeo"].get();
	wavesItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesItem->IndexCount = wavesItem->Geo->DrawArgs["grid"].IndexCount;
	wavesItem->StartIndexLocation = wavesItem->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesItem->BaseVertexLocation = wavesItem->Geo->DrawArgs["grid"].BaseVertexLocation;
	wavesItem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&wavesItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	m_RenderableLayer[(int)RenderLayer::Transparent].push_back(wavesItem.get());
	m_WavesRenderable = wavesItem.get();

	auto gridItem = std::make_unique<Renderable>();
	gridItem->ObjCBIndex = 1;
	gridItem->Mat = m_Materials["grass"].get();
	gridItem->Geo = m_Geometries["landGeo"].get();
	gridItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridItem->IndexCount = gridItem->Geo->DrawArgs["grid"].IndexCount;
	gridItem->StartIndexLocation = gridItem->Geo->DrawArgs["grid"].StartIndexLocation;
	gridItem->BaseVertexLocation = gridItem->Geo->DrawArgs["grid"].BaseVertexLocation;
	gridItem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridItem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	m_RenderableLayer[(int)RenderLayer::Opaque].push_back(gridItem.get());

	auto boxItem = std::make_unique<Renderable>();
	boxItem->ObjCBIndex = 2;
	boxItem->Mat = m_Materials["wirefence"].get();
	boxItem->Geo = m_Geometries["boxGeo"].get();
	boxItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxItem->IndexCount = boxItem->Geo->DrawArgs["box"].IndexCount;
	boxItem->StartIndexLocation = boxItem->Geo->DrawArgs["box"].StartIndexLocation;
	boxItem->BaseVertexLocation = boxItem->Geo->DrawArgs["box"].BaseVertexLocation;
	XMStoreFloat4x4(&boxItem->World, XMMatrixTranslation(3.0f, 2.0f, -9.0f));
	m_RenderableLayer[(int)RenderLayer::AlphaTested].push_back(boxItem.get());

	auto circleItem = std::make_unique<Renderable>();
	circleItem->ObjCBIndex = 3;
	circleItem->Mat = m_Materials["bricks"].get();
	circleItem->Geo = m_Geometries["circleGeo"].get();
	circleItem->PrimitiveType = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;
	circleItem->IndexCount = circleItem->Geo->DrawArgs["circle"].IndexCount;
	circleItem->StartIndexLocation = circleItem->Geo->DrawArgs["circle"].StartIndexLocation;
	circleItem->BaseVertexLocation = circleItem->Geo->DrawArgs["circle"].BaseVertexLocation;
	XMStoreFloat4x4(&circleItem->World, XMMatrixTranslation(2.0f, 10.0f, 2.0f));
	XMStoreFloat4x4(&circleItem->TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));
	m_RenderableLayer[(int)RenderLayer::Cylinder].push_back(circleItem.get());

	m_AllRenderables.push_back(std::move(wavesItem));
	m_AllRenderables.push_back(std::move(gridItem));
	m_AllRenderables.push_back(std::move(boxItem));
	m_AllRenderables.push_back(std::move(circleItem));
}

void CylinderApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Renderable*>& renderables)
{
	UINT objCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objCB = m_CurrFrameResource->ObjectCB->GetResource();
	auto matCB = m_CurrFrameResource->MaterialCB->GetResource();

	// For each render item..
	for (size_t i = 0; i < renderables.size(); ++i)
	{
		auto item = renderables[i];

		cmdList->IASetVertexBuffers(0, 1, &item->Geo->GetVertexBufferView());
		cmdList->IASetIndexBuffer(&item->Geo->GetIndexBufferView());
		cmdList->IASetPrimitiveTopology(item->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(item->Mat->DiffuseSrvHeapIndex, m_CbvSrvUavDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCB->GetGPUVirtualAddress() + item->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + item->Mat->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(2, matCBAddress);

		cmdList->DrawIndexedInstanced(item->IndexCount, 1, item->StartIndexLocation, item->BaseVertexLocation, 0);
	}
}

void CylinderApp::BuildFrameResources()
{
	for (int i = 0; i < g_NumFrameResources; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(m_d3dDevice.Get(),
			1, (UINT)m_AllRenderables.size(), (UINT)m_Materials.size(), m_Waves->GetVertexCount()));
	}
}

void CylinderApp::BuildDescriptorHeaps()
{
	// Create the SRV heap.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	// Fill out the heap with actual descriptors.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = m_Textures["grassTex"]->Resource;
	auto waterTex = m_Textures["waterTex"]->Resource;
	auto fenceTex = m_Textures["fenceTex"]->Resource;
	auto bricks   = m_Textures["bricks"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	srvDesc.Format = grassTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = bricks->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(bricks.Get(), &srvDesc, hDescriptor);
}

void CylinderApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Create root CBVs.
	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
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

void CylinderApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO opaqueDefines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestedDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	m_Shaders["StandardVS"] = D3DUtil::CompileShader(L"../Shaders/Standard_VS.hlsl", nullptr, "VS", "vs_5_0");
	m_Shaders["OpaquePS"] = D3DUtil::CompileShader(L"../Shaders/Opaque_PS.hlsl", opaqueDefines, "PS", "ps_5_0");
	m_Shaders["AlphaTestedPS"] = D3DUtil::CompileShader(L"../Shaders/AlphaTested_PS.hlsl", alphaTestedDefines, "PS", "ps_5_0");
	m_Shaders["CylinderVS"] = D3DUtil::CompileShader(L"../Shaders/Cylinder_VS.hlsl", nullptr, "VS", "vs_5_0");
	m_Shaders["CylinderGS"] = D3DUtil::CompileShader(L"../Shaders/Cylinder_GS.hlsl", nullptr, "GS", "gs_5_0");
	m_Shaders["CylinderPS"] = D3DUtil::CompileShader(L"../Shaders/Cylinder_PS.hlsl", alphaTestedDefines, "PS", "ps_5_0");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void CylinderApp::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	//
	// PSO for opaque objects.
	//
	opaquePsoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	opaquePsoDesc.pRootSignature = m_RootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["StandardVS"]->GetBufferPointer()),
		m_Shaders["StandardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["OpaquePS"]->GetBufferPointer()),
		m_Shaders["OpaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = m_BackBufferFormat;
	opaquePsoDesc.DSVFormat = m_DepthStencilFormat;
	opaquePsoDesc.SampleDesc.Count = m_msaaState ? m_msaaLevel : 1;
	opaquePsoDesc.SampleDesc.Quality = m_msaaState ? (m_msaaQuality - 1) : 0;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs["Opaque"])));

	//
	// PSO for transparent objects.
	//
	D3D12_RENDER_TARGET_BLEND_DESC transparentBlendDesc;
	transparentBlendDesc.BlendEnable = true;
	transparentBlendDesc.LogicOpEnable = false;
	transparentBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparentBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparentBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparentBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparentBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparentBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparentBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparentBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;
	transparentPsoDesc.BlendState.RenderTarget[0] = transparentBlendDesc;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&m_PSOs["Transparent"])));

	//
	// PSO for alpha tested objects.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["AlphaTestedPS"]->GetBufferPointer()),
		m_Shaders["AlphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&m_PSOs["AlphaTested"])));

	//
	// PSO for cylinder generating from a circle.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC cylinderPsoDesc = opaquePsoDesc;
	cylinderPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["CylinderVS"]->GetBufferPointer()),
		m_Shaders["CylinderVS"]->GetBufferSize()
	};
	cylinderPsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["CylinderGS"]->GetBufferPointer()),
		m_Shaders["CylinderGS"]->GetBufferSize()
	};
	cylinderPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_Shaders["CylinderPS"]->GetBufferPointer()),
		m_Shaders["CylinderPS"]->GetBufferSize()
	};
	cylinderPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	cylinderPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&cylinderPsoDesc, IID_PPV_ARGS(&m_PSOs["Cylinder"])));

	//
	// PSO for opaque wireframe objects.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	opaqueWireframePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["Opaque_Wireframe"])));
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> CylinderApp::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers. So just define them all up front
	// and keep them available as part of the root signature.

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap
	(
		0,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP		// addressW
	);

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp
	(
		1,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT,		// filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP		// addressW
	);

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap
	(
		2,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,	// filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP		// addressW
	);

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp
	(
		3,									// shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,	// filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP	// addressW
	);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap
	(
		4,									// shaderRegister
		D3D12_FILTER_ANISOTROPIC,			// filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,	// addressW
		0.0f,								// mipLODBias
		8									// maxAnisotropy
	);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp
	(
		5,									// shaderRegister
		D3D12_FILTER_ANISOTROPIC,			// filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// addressW
		0.0f,								// mipLODBias
		8									// maxAnisotropy
	);

	return
	{
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp
	};
}