#include "StencilApp.h"

StencilApp::StencilApp(HINSTANCE hInst)
	: D3DApp(hInst)
{

}

StencilApp::~StencilApp()
{

}

bool StencilApp::Initialize()
{
	if (!D3DApp::Initialize())
	{
		return false;
	}

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	LoadTextures();
	BuildRoomGeometry();
	BuildSkullGeometry();
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

void StencilApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, proj);
}

void StencilApp::Update(const GameTimer& timer)
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
	UpdateMaterialCBs(timer);
	UpdateMainPassCB(timer);
	UpdateReflectedPassCB(timer);
}

void StencilApp::Draw(const GameTimer& timer)
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

	UINT passCBByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	// Draw opaque render items.
	auto passCB = m_CurrFrameResource->PassCB->GetResource();
	m_CommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Opaque]);

	// Mark the visible mirror pixels in the stencil buffer with the value 1.
	m_CommandList->OMSetStencilRef(1);
	m_CommandList->SetPipelineState(m_PSOs["Mark_Stencil_Mirrors"].Get());
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Mirrors]);

	// Draw the reflection into the mirror only (only for pixels where the stencil buffer is 1).
	// Note that we must supply a different per-pass constant buffer (one with the lights reflected).
	m_CommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress() + 1 * passCBByteSize);
	m_CommandList->SetPipelineState(m_PSOs["Stencil_Reflections"].Get());
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Reflected]);

	// Restore main pass constants and stencil ref.
	m_CommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
	m_CommandList->OMSetStencilRef(0);

	// Draw mirror with transparency so reflection blends through.
	m_CommandList->SetPipelineState(m_PSOs["Transparent"].Get());
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Transparent]);

	// Draw shadows
	m_CommandList->SetPipelineState(m_PSOs["Shadow"].Get());
	DrawRenderItems(m_CommandList.Get(), m_RenderableLayer[(int)RenderLayer::Shadow]);

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

void StencilApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = m_InitialMousePos.x = x;
	m_LastMousePos.y = m_InitialMousePos.y = y;

	SetCapture(m_AppWnd);
	ShowCursor(false);
}

void StencilApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
	ClientToScreen(m_AppWnd, &m_InitialMousePos);
	SetCursorPos(m_InitialMousePos.x, m_InitialMousePos.y);
	ShowCursor(true);
}

void StencilApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void StencilApp::OnKeyboardInput(const GameTimer& timer)
{
	if (GetAsyncKeyState('1') & 0x8000)
		m_IsWireframe = true;
	else
		m_IsWireframe = false;

	//
	// Allow user to move skull.
	//

	const float dt = timer.DeltaTime();

	if (GetAsyncKeyState('A') & 0x8000)
		m_SkullTranslation.x -= 1.0f * dt;
	if (GetAsyncKeyState('D') & 0x8000)
		m_SkullTranslation.x += 1.0f * dt;
	if (GetAsyncKeyState('W') & 0x8000)
		m_SkullTranslation.y += 1.0f * dt;
	if (GetAsyncKeyState('S') & 0x8000)
		m_SkullTranslation.y -= 1.0f * dt;

	// Don't let user move below ground plane.
	m_SkullTranslation.y = MathHelper::Max(m_SkullTranslation.y, 0.0f);

	// Update the new world matrix.
	XMMATRIX skullRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(m_SkullTranslation.x, m_SkullTranslation.y, m_SkullTranslation.z);
	XMMATRIX skullWorld = skullRotate * skullScale * skullOffset;
	XMStoreFloat4x4(&m_SkullItem->World, skullWorld);

	// Update the reflection world matrix.
	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);
	XMStoreFloat4x4(&m_ReflectedSkullItem->World, skullWorld * R);

	// Update shadow world matrix.
	XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
	XMVECTOR toMainLight = -XMLoadFloat3(&m_MainPassCB.Lights[0].Direction);
	XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
	XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);
	XMStoreFloat4x4(&m_ShadowedSkullItem->World, skullWorld * S * shadowOffsetY);

	m_SkullItem->NumFrameDirty = g_NumFrameResources;
	m_ReflectedSkullItem->NumFrameDirty = g_NumFrameResources;
	m_ShadowedSkullItem->NumFrameDirty = g_NumFrameResources;
}

void StencilApp::UpdateCamera(const GameTimer& timer)
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

void StencilApp::UpdateObjectCBs(const GameTimer& timer)
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

void StencilApp::UpdateMaterialCBs(const GameTimer& timer)
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

void StencilApp::UpdateMainPassCB(const GameTimer& timer)
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
	m_MainPassCB.Lights[0].Strength  = { 0.6f, 0.6f, 0.6f };
	m_MainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	m_MainPassCB.Lights[1].Strength  = { 0.3f, 0.3f, 0.3f };
	m_MainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	m_MainPassCB.Lights[2].Strength  = { 0.15f, 0.15f, 0.15f };

	// Main pass stored in index 2
	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}

void StencilApp::UpdateReflectedPassCB(const GameTimer& timer)
{
	m_ReflectedPassCB = m_MainPassCB;

	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMMATRIX R = XMMatrixReflect(mirrorPlane);

	// Reflect the lighting.
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&m_MainPassCB.Lights[i].Direction);
		XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&m_ReflectedPassCB.Lights[i].Direction, reflectedLightDir);
	}

	// Reflected pass stored in index 1
	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(1, m_ReflectedPassCB);
}

float StencilApp::GetHillsHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 StencilApp::GetHillsNormal(float x, float z) const
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

void StencilApp::LoadTextures()
{
	auto bricksTex = std::make_unique<Texture>();
	bricksTex->Name = "bricksTex";
	bricksTex->Filename = L"Textures/bricks3.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), bricksTex->Filename.c_str(),
		bricksTex->Resource, bricksTex->UploadHeap));

	auto checkboardTex = std::make_unique<Texture>();
	checkboardTex->Name = "checkboardTex";
	checkboardTex->Filename = L"Textures/checkboard.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), checkboardTex->Filename.c_str(),
		checkboardTex->Resource, checkboardTex->UploadHeap));

	auto iceTex = std::make_unique<Texture>();
	iceTex->Name = "iceTex";
	iceTex->Filename = L"Textures/ice.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), iceTex->Filename.c_str(),
		iceTex->Resource, iceTex->UploadHeap));

	auto skullTex = std::make_unique<Texture>();
	skullTex->Name = "skullTex";
	skullTex->Filename = L"Textures/white1x1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), skullTex->Filename.c_str(),
		skullTex->Resource, skullTex->UploadHeap));

	m_Textures[bricksTex->Name] = std::move(bricksTex);
	m_Textures[checkboardTex->Name] = std::move(checkboardTex);
	m_Textures[iceTex->Name] = std::move(iceTex);
	m_Textures[skullTex->Name] = std::move(skullTex);
}

void StencilApp::BuildRoomGeometry()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/

	std::array<Vertex, 20> vertices =
	{
		// Floor: Observe we tile texture coordinates.
		Vertex(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f), // 0 
		Vertex(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
		Vertex(7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f),
		Vertex(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f),

		// Wall: Observe we tile texture coordinates, and that we
		// leave a gap in the middle for the mirror.
		Vertex(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 4
		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f),
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f),

		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f), // 8 
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f),
		Vertex(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f),

		Vertex(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 12
		Vertex(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f),
		Vertex(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f),

		// Mirror
		Vertex(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f), // 16
		Vertex(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),
		Vertex(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f),
		Vertex(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f)
	};

	std::array<std::int16_t, 30> indices =
	{
		// Floor
		0, 1, 2,
		0, 2, 3,

		// Walls
		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		// Mirror
		16, 17, 18,
		16, 18, 19
	};

	SubMeshGeometry floorSubmesh;
	floorSubmesh.IndexCount = 6;
	floorSubmesh.StartIndexLocation = 0;
	floorSubmesh.BaseVertexLocation = 0;

	SubMeshGeometry wallSubmesh;
	wallSubmesh.IndexCount = 18;
	wallSubmesh.StartIndexLocation = 6;
	wallSubmesh.BaseVertexLocation = 0;

	SubMeshGeometry mirrorSubmesh;
	mirrorSubmesh.IndexCount = 6;
	mirrorSubmesh.StartIndexLocation = 24;
	mirrorSubmesh.BaseVertexLocation = 0;

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "roomGeo";

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

	geo->DrawArgs["wall"] = wallSubmesh;
	geo->DrawArgs["floor"] = floorSubmesh;
	geo->DrawArgs["mirror"] = mirrorSubmesh;

	m_Geometries[geo->Name] = std::move(geo);
}

void StencilApp::BuildSkullGeometry()
{
	GeometryGenerator::MeshData skull = GeometryGenerator::LoadModel("Models/skull.txt");

	std::vector<Vertex> vertices(skull.Vertices.size());
	for (size_t i = 0; i < skull.Vertices.size(); ++i)
	{
		vertices[i].Pos = skull.Vertices[i].Position;
		vertices[i].Normal = skull.Vertices[i].Normal;
		vertices[i].TexCoord = skull.Vertices[i].TexCoord;
	}

	std::vector<std::uint16_t> indices = skull.GetIndices16();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "skullGeo";

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
	submesh.IndexCount = (UINT)skull.Indices32.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	geo->DrawArgs["skull"] = submesh;

	m_Geometries[geo->Name] = std::move(geo);
}

void StencilApp::BuildMaterials()
{
	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->MatCBIndex = 0;
	bricks->DiffuseSrvHeapIndex = 0;
	bricks->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = XMFLOAT3(0.5f, 0.5f, 0.5f);
	bricks->Roughness = 0.25f;

	auto checkerTile = std::make_unique<Material>();
	checkerTile->Name = "checkerTile";
	checkerTile->MatCBIndex = 1;
	checkerTile->DiffuseSrvHeapIndex = 1;
	checkerTile->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	checkerTile->FresnelR0 = XMFLOAT3(0.07f, 0.07f, 0.07f);
	checkerTile->Roughness = 0.3f;

	auto icemirror = std::make_unique<Material>();
	icemirror->Name = "icemirror";
	icemirror->MatCBIndex = 2;
	icemirror->DiffuseSrvHeapIndex = 2;
	icemirror->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
	icemirror->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	icemirror->Roughness = 0.5f;

	auto skull = std::make_unique<Material>();
	skull->Name = "skull";
	skull->MatCBIndex = 3;
	skull->DiffuseSrvHeapIndex = 3;
	skull->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skull->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	skull->Roughness = 0.3f;

	auto shadow = std::make_unique<Material>();
	shadow->Name = "shadow";
	shadow->MatCBIndex = 4;
	shadow->DiffuseSrvHeapIndex = 3;
	shadow->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	shadow->FresnelR0 = XMFLOAT3(0.001f, 0.001f, 0.001f);
	shadow->Roughness = 0.0f;

	m_Materials["bricks"] = std::move(bricks);
	m_Materials["checkerTile"] = std::move(checkerTile);
	m_Materials["icemirror"] = std::move(icemirror);
	m_Materials["skull"] = std::move(skull);
	m_Materials["shadow"] = std::move(shadow);
}

void StencilApp::BuildRenderItems()
{
	auto floorItem = std::make_unique<Renderable>();
	floorItem->ObjCBIndex = 0;
	floorItem->Mat = m_Materials["checkerTile"].get();
	floorItem->Geo = m_Geometries["roomGeo"].get();
	floorItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	floorItem->IndexCount = floorItem->Geo->DrawArgs["floor"].IndexCount;
	floorItem->StartIndexLocation = floorItem->Geo->DrawArgs["floor"].StartIndexLocation;
	floorItem->BaseVertexLocation = floorItem->Geo->DrawArgs["floor"].BaseVertexLocation;
	floorItem->World = MathHelper::Identity4x4();
	floorItem->TexTransform = MathHelper::Identity4x4();
	m_RenderableLayer[(int)RenderLayer::Opaque].push_back(floorItem.get());

	auto wallsItem = std::make_unique<Renderable>();
	wallsItem->ObjCBIndex = 1;
	wallsItem->Mat = m_Materials["bricks"].get();
	wallsItem->Geo = m_Geometries["roomGeo"].get();
	wallsItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wallsItem->IndexCount = wallsItem->Geo->DrawArgs["wall"].IndexCount;
	wallsItem->StartIndexLocation = wallsItem->Geo->DrawArgs["wall"].StartIndexLocation;
	wallsItem->BaseVertexLocation = wallsItem->Geo->DrawArgs["wall"].BaseVertexLocation;
	wallsItem->World = MathHelper::Identity4x4();
	wallsItem->TexTransform = MathHelper::Identity4x4();
	m_RenderableLayer[(int)RenderLayer::Opaque].push_back(wallsItem.get());

	auto skullItem = std::make_unique<Renderable>();
	skullItem->ObjCBIndex = 2;
	skullItem->Mat = m_Materials["skull"].get();
	skullItem->Geo = m_Geometries["skullGeo"].get();
	skullItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skullItem->IndexCount = skullItem->Geo->DrawArgs["skull"].IndexCount;
	skullItem->StartIndexLocation = skullItem->Geo->DrawArgs["skull"].StartIndexLocation;
	skullItem->BaseVertexLocation = skullItem->Geo->DrawArgs["skull"].BaseVertexLocation;
	skullItem->World = MathHelper::Identity4x4();
	skullItem->TexTransform = MathHelper::Identity4x4();
	m_SkullItem = skullItem.get();
	m_RenderableLayer[(int)RenderLayer::Opaque].push_back(skullItem.get());

	// Reflected skull will have different world matrix, so it needs to be its own render item.
	auto reflectedSkullItem = std::make_unique<Renderable>();
	*reflectedSkullItem = *skullItem;
	reflectedSkullItem->ObjCBIndex = 3;
	m_ReflectedSkullItem = reflectedSkullItem.get();
	m_RenderableLayer[(int)RenderLayer::Reflected].push_back(reflectedSkullItem.get());

	// Shadowed skull will have different world matrix, so it needs to be its own render item.
	auto shadowedSkullItem = std::make_unique<Renderable>();
	*shadowedSkullItem = *skullItem;
	shadowedSkullItem->ObjCBIndex = 4;
	shadowedSkullItem->Mat = m_Materials["shadow"].get();
	m_ShadowedSkullItem = shadowedSkullItem.get();
	m_RenderableLayer[(int)RenderLayer::Shadow].push_back(shadowedSkullItem.get());

	auto mirrorItem = std::make_unique<Renderable>();
	mirrorItem->ObjCBIndex = 5;
	mirrorItem->Mat = m_Materials["icemirror"].get();
	mirrorItem->Geo = m_Geometries["roomGeo"].get();
	mirrorItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mirrorItem->IndexCount = mirrorItem->Geo->DrawArgs["mirror"].IndexCount;
	mirrorItem->StartIndexLocation = mirrorItem->Geo->DrawArgs["mirror"].StartIndexLocation;
	mirrorItem->BaseVertexLocation = mirrorItem->Geo->DrawArgs["mirror"].BaseVertexLocation;
	mirrorItem->World = MathHelper::Identity4x4();
	mirrorItem->TexTransform = MathHelper::Identity4x4();
	m_RenderableLayer[(int)RenderLayer::Mirrors].push_back(mirrorItem.get());
	m_RenderableLayer[(int)RenderLayer::Transparent].push_back(mirrorItem.get());

	m_AllRenderables.push_back(std::move(floorItem));
	m_AllRenderables.push_back(std::move(wallsItem));
	m_AllRenderables.push_back(std::move(skullItem));
	m_AllRenderables.push_back(std::move(reflectedSkullItem));
	m_AllRenderables.push_back(std::move(shadowedSkullItem));
	m_AllRenderables.push_back(std::move(mirrorItem));
}

void StencilApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Renderable*>& renderables)
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

void StencilApp::BuildFrameResources()
{
	for (int i = 0; i < g_NumFrameResources; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(m_d3dDevice.Get(),
			2, (UINT)m_AllRenderables.size(), (UINT)m_Materials.size()));
	}
}

void StencilApp::BuildDescriptorHeaps()
{
	// Create the SRV heap.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvDescriptorHeap)));

	// Fill out the heap with actual descriptors.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTex     = m_Textures["bricksTex"]->Resource;
	auto checkboardTex = m_Textures["checkboardTex"]->Resource;
	auto iceTex        = m_Textures["iceTex"]->Resource;
	auto skullTex      = m_Textures["skullTex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	srvDesc.Format = bricksTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = checkboardTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(checkboardTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = iceTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(iceTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);

	srvDesc.Format = skullTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(skullTex.Get(), &srvDesc, hDescriptor);
}

void StencilApp::BuildRootSignature()
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

void StencilApp::BuildShadersAndInputLayout()                       
{
	const D3D_SHADER_MACRO opaqueDefines[] = 
	{
		"FOG", "0",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestedDefines[] = 
	{
		"FOG", "0",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	std::wstring projectName = PROJ_NAME;
	m_Shaders["StandardVS"] = D3DUtil::CompileShader(L"Shaders/" + projectName + L"/Standard_VS.hlsl", nullptr, "VS", "vs_5_0");
	m_Shaders["OpaquePS"] = D3DUtil::CompileShader(L"Shaders/" + projectName + L"/Opaque_PS.hlsl", opaqueDefines, "PS", "ps_5_0");
	m_Shaders["AlphaTestedPS"] = D3DUtil::CompileShader(L"Shaders/" + projectName + L"/AlphaTested_PS.hlsl", alphaTestedDefines, "PS", "ps_5_0");

	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void StencilApp::BuildPSOs()
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
	// PSO for marking stencil mirrors.
	//

	CD3DX12_BLEND_DESC mirrorBS(D3D12_DEFAULT);
	mirrorBS.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC mirrorDSS;
	mirrorDSS.DepthEnable = true;
	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSS.StencilEnable = true;
	mirrorDSS.StencilReadMask = 0xff;
	mirrorDSS.StencilWriteMask = 0xff;

	mirrorDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	// We're not rendering backfacing polygons, so these settings do not matter.
	mirrorDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC markMirrorsPsoDesc = opaquePsoDesc;
	markMirrorsPsoDesc.BlendState = mirrorBS;
	markMirrorsPsoDesc.DepthStencilState = mirrorDSS;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&markMirrorsPsoDesc, IID_PPV_ARGS(&m_PSOs["Mark_Stencil_Mirrors"])));

	//
	// PSO for stencil reflections.
	//

	D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
	reflectionsDSS.DepthEnable = true;
	reflectionsDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionsDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionsDSS.StencilEnable = true;
	reflectionsDSS.StencilReadMask = 0xff;
	reflectionsDSS.StencilWriteMask = 0xff;

	reflectionsDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We're not rendering backfacing polygons, so these settings do not matter.
	reflectionsDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionsDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC reflectionsPsoDesc = opaquePsoDesc;
	reflectionsPsoDesc.DepthStencilState = reflectionsDSS;
	reflectionsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	reflectionsPsoDesc.RasterizerState.FrontCounterClockwise = true;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&reflectionsPsoDesc, IID_PPV_ARGS(&m_PSOs["Stencil_Reflections"])));

	//
	// PSO for shadow objects.
	//

	// We're going to draw shadows with transparency, so base it off the transparency description.
	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We're not rendering backfacing polygons, so these settings do not matter.
	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = transparentPsoDesc;
	shadowPsoDesc.DepthStencilState = shadowDSS;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&m_PSOs["Shadow"])));

	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	opaqueWireframePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&m_PSOs["Opaque_Wireframe"])));
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> StencilApp::GetStaticSamplers()
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
