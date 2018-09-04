#pragma once

#include <D3DApp.h>
#include <D3DUtil.h>
#include <GameTimer.h>
#include <UploadBuffer.h>
#include <MathHelper.h>
#include "FrameResource.h"

// Num of frame resources.
const int g_NumFrameResources = 3;

enum class RenderLayer : int
{
	Opaque = 0,
	Count
};

// Lightweight structure stores parameters to draw a shape. This will
// vary from app-to-app.
struct Renderable
{
	Renderable() = default;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource. Thus, when we modify object data we should set
	// NumFrameDirty = g_NumFrameResources so that each frame resource gets the update.
	int NumFrameDirty = g_NumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	// Render item geometry.
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int  BaseVertexLocation = 0;
};

class Application : public D3DApp
{
public:
	Application(HINSTANCE hInst);
	Application(const Application& rhs) = delete;
	Application& operator=(const Application& rhs) = delete;
	~Application();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& timer) override;
	virtual void Draw(const GameTimer& timer) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

	void OnKeyboardInput(const GameTimer& timer);

	void UpdateCamera(const GameTimer& timer);
	void UpdateObjectCBs(const GameTimer& timer);
	void UpdateMainPassCB(const GameTimer& timer);

	void BuildIcosahedronPatchGeometry();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Renderable*>& renderables);
	void BuildFrameResources();

	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSOs();

private:
	// List of all the render items.
	std::vector<std::unique_ptr<Renderable>> m_AllRenderables;

	// Render items divided by PSO.
	std::vector<Renderable*> m_RenderableLayer[(int)RenderLayer::Count];

	// Frame Resources.
	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
	FrameResource* m_CurrFrameResource = nullptr;
	int m_CurrFrameResourceIndex = 0;

	// Root Signature.
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;

	// Shaders and InputLayout.
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_Shaders;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// Pipeline State Objects (PSOs).
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	// Geometry (vertex/index buffers).
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;

	// Pass constant buffer.
	PassConstants m_MainPassCB;

	// Demo related members.
	XMFLOAT3   m_EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 m_View  = MathHelper::Identity4x4();
	XMFLOAT4X4 m_Proj  = MathHelper::Identity4x4();

	float m_Theta = 1.5f * XM_PI;
	float m_Phi = 0.2f * XM_PI;
	float m_Radius = 15.0f;

	POINT m_InitialMousePos;
	POINT m_LastMousePos;
};