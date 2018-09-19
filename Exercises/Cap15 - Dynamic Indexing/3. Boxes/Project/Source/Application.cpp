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

	m_Camera.SetPosition(0.0f, 2.0f, -15.0f);

	LoadTextures();
	BuildBoxesGeometry();
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

void Application::OnResize()
{
	D3DApp::OnResize();

	m_Camera.SetLens(0.25f * MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
}

void Application::Update(const GameTimer& timer)
{
	OnKeyboardInput(timer);

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
	UpdateMaterialBuffer(timer);
}

void Application::Draw(const GameTimer& timer)
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
	m_CommandList->ClearRenderTargetView(GetCurrentBackBufferView(), Colors::Black, 0, nullptr);
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
	m_CommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	// Bind all the materials used in this scene.
	// For structured buffers, we can bypass the heap and set as a root descriptor.
	auto matBuffer = m_CurrFrameResource->MaterialBuffer->GetResource();
	m_CommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

	// Bind all the textures used in this scene.
	// Observe that we only have to specify the first descriptor in the table.
	// The root signature knows how many descriptors are expected in the table.
	m_CommandList->SetGraphicsRootDescriptorTable(3, m_SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// Draw render items.
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Opaque]);

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

		m_Camera.Pitch(dy);
		m_Camera.RotateY(dx);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		m_Camera.Roll(dx);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

void Application::OnKeyboardInput(const GameTimer& timer)
{
	const float dt = timer.DeltaTime();

	if (GetAsyncKeyState('1') & 0x8000)
		m_IsWireframe = true;
	else
		m_IsWireframe = false;

	if (GetAsyncKeyState('W') & 0x8000)
		m_Camera.Walk(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		m_Camera.Walk(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		m_Camera.Strafe(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		m_Camera.Strafe(10.0f * dt);

	m_Camera.UpdateViewMatrix();
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
			XMMATRIX texTransform = XMLoadFloat4x4(&item->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.MaterialIndex = item->Mat->MatCBIndex;

			currObjectCB->CopyData(item->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			item->NumFrameDirty--;
		}
	}
}

void Application::UpdateMainPassCB(const GameTimer& timer)
{
	XMMATRIX view = m_Camera.GetView();
	XMMATRIX proj = m_Camera.GetProj();
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

	m_MainPassCB.EyePosW = m_Camera.GetPosition3f();
	m_MainPassCB.RenderTargetSize = XMFLOAT2((float)m_ClientWidth, (float)m_ClientHeight);
	m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_ClientWidth, 1.0f / m_ClientHeight);
	m_MainPassCB.NearZ = 1.0f;
	m_MainPassCB.FarZ = 1000.0f;
	m_MainPassCB.TotalTime = timer.TotalTime();
	m_MainPassCB.DeltaTime = timer.DeltaTime();

	m_MainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	m_MainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[0].Strength  = { 0.9f, 0.9f, 0.9f };
	m_MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[1].Strength  = { 0.5f, 0.5f, 0.5f };
	m_MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_MainPassCB.Lights[2].Strength  = { 0.2f, 0.2f, 0.2f };

	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void Application::UpdateMaterialBuffer(const GameTimer& timer)
{
	auto currMaterialBuffer = m_CurrFrameResource->MaterialBuffer.get();
	for (auto& e : m_Materials)
	{
		// Only update the buffer if the constants have changed. If the buffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void Application::LoadTextures()
{
	auto box1 = std::make_unique<Texture>();
	box1->Name = "box1";
	box1->Filename = L"Textures/box1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), box1->Filename.c_str(),
		box1->Resource, box1->UploadHeap));

	auto box2 = std::make_unique<Texture>();
	box2->Name = "box2";
	box2->Filename = L"Textures/box2.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), box2->Filename.c_str(),
		box2->Resource, box2->UploadHeap));

	auto box3 = std::make_unique<Texture>();
	box3->Name = "box3";
	box3->Filename = L"Textures/box3.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), box3->Filename.c_str(),
		box3->Resource, box3->UploadHeap));

	auto box4 = std::make_unique<Texture>();
	box4->Name = "box4";
	box4->Filename = L"Textures/box4.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), box4->Filename.c_str(),
		box4->Resource, box4->UploadHeap));

	auto box5 = std::make_unique<Texture>();
	box5->Name = "box5";
	box5->Filename = L"Textures/box5.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), box5->Filename.c_str(),
		box5->Resource, box5->UploadHeap));

	m_Textures[box1->Name] = std::move(box1);
	m_Textures[box2->Name] = std::move(box2);
	m_Textures[box3->Name] = std::move(box3);
	m_Textures[box4->Name] = std::move(box4);
	m_Textures[box5->Name] = std::move(box5);
}

void Application::BuildBoxesGeometry()
{
	GeometryGenerator::MeshData box = GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f, 3);

	//
	// We are concatenating all the geometry into one big vertex/index buffer. So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT box1VertexOffset = 0;
	UINT box2VertexOffset = (UINT)box.Vertices.size();
	UINT box3VertexOffset = box2VertexOffset + (UINT)box.Vertices.size();
	UINT box4VertexOffset = box3VertexOffset + (UINT)box.Vertices.size();
	UINT box5VertexOffset = box4VertexOffset + (UINT)box.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT box1IndexOffset = 0;
	UINT box2IndexOffset = (UINT)box.Indices32.size();
	UINT box3IndexOffset = box2IndexOffset + (UINT)box.Indices32.size();
	UINT box4IndexOffset = box3IndexOffset + (UINT)box.Indices32.size();
	UINT box5IndexOffset = box4IndexOffset + (UINT)box.Indices32.size();

	// Define the SubMeshGeometry that cover different
	// regions of the vertex/index buffers.
	SubMeshGeometry box1Submesh;
	box1Submesh.IndexCount = (UINT)box.Indices32.size();
	box1Submesh.StartIndexLocation = box1IndexOffset;
	box1Submesh.BaseVertexLocation = box1VertexOffset;

	SubMeshGeometry box2Submesh;
	box2Submesh.IndexCount = (UINT)box.Indices32.size();
	box2Submesh.StartIndexLocation = box2IndexOffset;
	box2Submesh.BaseVertexLocation = box2VertexOffset;

	SubMeshGeometry box3Submesh;
	box3Submesh.IndexCount = (UINT)box.Indices32.size();
	box3Submesh.StartIndexLocation = box3IndexOffset;
	box3Submesh.BaseVertexLocation = box3VertexOffset;

	SubMeshGeometry box4Submesh;
	box4Submesh.IndexCount = (UINT)box.Indices32.size();
	box4Submesh.StartIndexLocation = box4IndexOffset;
	box4Submesh.BaseVertexLocation = box4VertexOffset;

	SubMeshGeometry box5Submesh;
	box5Submesh.IndexCount = (UINT)box.Indices32.size();
	box5Submesh.StartIndexLocation = box5IndexOffset;
	box5Submesh.BaseVertexLocation = box5VertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount = 5 * box.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexCoord = box.Vertices[i].TexCoord;
		vertices[k].DiffuseMapIndex = 0;
	}

	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexCoord = box.Vertices[i].TexCoord;
		vertices[k].DiffuseMapIndex = 1;
	}

	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexCoord = box.Vertices[i].TexCoord;
		vertices[k].DiffuseMapIndex = 2;
	}

	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexCoord = box.Vertices[i].TexCoord;
		vertices[k].DiffuseMapIndex = 3;
	}

	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexCoord = box.Vertices[i].TexCoord;
		vertices[k].DiffuseMapIndex = 4;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxesGeo";

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


	geo->DrawArgs["box1"] = box1Submesh;
	geo->DrawArgs["box2"] = box2Submesh;
	geo->DrawArgs["box3"] = box3Submesh;
	geo->DrawArgs["box4"] = box4Submesh;
	geo->DrawArgs["box5"] = box5Submesh;

	m_Geometries[geo->Name] = std::move(geo);
}

void Application::BuildMaterials()
{
	auto boxes = std::make_unique<Material>();
	boxes->Name = "boxes";
	boxes->MatCBIndex = 0;
	boxes->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	boxes->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	boxes->Roughness = 0.1f;

	m_Materials["boxes"] = std::move(boxes);
}

void Application::BuildRenderItems()
{
	auto box1 = std::make_unique<Renderable>();
	box1->ObjCBIndex = 0;
	box1->Mat = m_Materials["boxes"].get();
	box1->Geo = m_Geometries["boxesGeo"].get();
	box1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	box1->IndexCount = box1->Geo->DrawArgs["box1"].IndexCount;
	box1->StartIndexLocation = box1->Geo->DrawArgs["box1"].StartIndexLocation;
	box1->BaseVertexLocation = box1->Geo->DrawArgs["box1"].BaseVertexLocation;
	XMStoreFloat4x4(&box1->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(-5.0f, 1.0f, -5.0f));
	XMStoreFloat4x4(&box1->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	m_AllRenderables.push_back(std::move(box1));

	auto box2 = std::make_unique<Renderable>();
	box2->ObjCBIndex = 1;
	box2->Mat = m_Materials["boxes"].get();
	box2->Geo = m_Geometries["boxesGeo"].get();
	box2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	box2->IndexCount = box2->Geo->DrawArgs["box2"].IndexCount;
	box2->StartIndexLocation = box2->Geo->DrawArgs["box2"].StartIndexLocation;
	box2->BaseVertexLocation = box2->Geo->DrawArgs["box2"].BaseVertexLocation;
	XMStoreFloat4x4(&box2->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(5.0f, 1.0f, -5.0f));
	XMStoreFloat4x4(&box2->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	m_AllRenderables.push_back(std::move(box2));

	auto box3 = std::make_unique<Renderable>();
	box3->ObjCBIndex = 2;
	box3->Mat = m_Materials["boxes"].get();
	box3->Geo = m_Geometries["boxesGeo"].get();
	box3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	box3->IndexCount = box3->Geo->DrawArgs["box3"].IndexCount;
	box3->StartIndexLocation = box3->Geo->DrawArgs["box3"].StartIndexLocation;
	box3->BaseVertexLocation = box3->Geo->DrawArgs["box3"].BaseVertexLocation;
	XMStoreFloat4x4(&box3->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 1.0f, 0.0f));
	XMStoreFloat4x4(&box3->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	m_AllRenderables.push_back(std::move(box3));

	auto box4 = std::make_unique<Renderable>();
	box4->ObjCBIndex = 3;
	box4->Mat = m_Materials["boxes"].get();
	box4->Geo = m_Geometries["boxesGeo"].get();
	box4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	box4->IndexCount = box4->Geo->DrawArgs["box4"].IndexCount;
	box4->StartIndexLocation = box4->Geo->DrawArgs["box4"].StartIndexLocation;
	box4->BaseVertexLocation = box4->Geo->DrawArgs["box4"].BaseVertexLocation;
	XMStoreFloat4x4(&box4->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(-5.0f, 1.0f, 5.0f));
	XMStoreFloat4x4(&box4->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	m_AllRenderables.push_back(std::move(box4));

	auto box5 = std::make_unique<Renderable>();
	box5->ObjCBIndex = 4;
	box5->Mat = m_Materials["boxes"].get();
	box5->Geo = m_Geometries["boxesGeo"].get();
	box5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	box5->IndexCount = box5->Geo->DrawArgs["box5"].IndexCount;
	box5->StartIndexLocation = box5->Geo->DrawArgs["box5"].StartIndexLocation;
	box5->BaseVertexLocation = box5->Geo->DrawArgs["box5"].BaseVertexLocation;
	XMStoreFloat4x4(&box5->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(5.0f, 1.0f, 5.0f));
	XMStoreFloat4x4(&box5->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	m_AllRenderables.push_back(std::move(box5));

	// All the render items are opaque.
	for (auto& item : m_AllRenderables)
		m_RenderableLayer[(int)RenderLayer::Opaque].push_back(item.get());
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
		m_FrameResources.push_back(std::make_unique<FrameResource>(m_d3dDevice.Get(),
			1, (UINT)m_AllRenderables.size(), (UINT)m_Materials.size()));
	}
}

void Application::BuildDescriptorHeaps()
{
	// Create the SRV heap.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = m_Textures.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	// Fill out the heap with actual descriptors.

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor;
	int offset = 0;

	for (auto it = m_Textures.begin(); it != m_Textures.end(); ++it)
	{
		// Offset the handle to the next descriptor.
		hDescriptor = m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		hDescriptor.Offset(offset, m_CbvSrvUavDescriptorSize);

		auto tex = it->second->Resource;
		srvDesc.Format = tex->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
		m_d3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc, hDescriptor);

		offset++;
	}
}

void Application::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_Textures.size(), 0, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Create root CBVs.
	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsShaderResourceView(0, 1);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

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

void Application::BuildShadersAndInputLayout()                       
{
	m_Shaders["StandardVS"] = D3DUtil::LoadBinary(L"Shaders/Standard_VS.cso");
	m_Shaders["OpaquePS"] = D3DUtil::LoadBinary(L"Shaders/Opaque_PS.cso");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "INDEX",    0, DXGI_FORMAT_R32_UINT,        0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void Application::BuildPSOs()
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
	// PSO for opaque wireframe objects.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	opaqueWireframePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["Opaque_Wireframe"])));
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Application::GetStaticSamplers()
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
