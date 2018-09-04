#pragma once

#include <D3DApp.h>
#include <D3DUtil.h>
#include <GameTimer.h>
#include <UploadBuffer.h>
#include <MathHelper.h>
#include "FrameResource.h"
#include "GpuWaves.h"
#include "SobelFilter.h"
#include "RenderTarget.h"

// Num of frame resources.
const int g_NumFrameResources = 3;

enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	GpuWaves,
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

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Used for GPU waves render items.
	XMFLOAT2 DisplacementMapTexelSize = { 1.0f, 1.0f };
	float GridSpatialStep = 1.0f;

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource. Thus, when we modify object data we should set
	// NumFrameDirty = g_NumFrameResources so that each frame resource gets the update.
	int NumFrameDirty = g_NumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	// Render item material.
	Material* Mat = nullptr;

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
	virtual void CreateRtvAndDsvDescriptorHeaps() override;
	virtual void OnResize() override;
	virtual void Update(const GameTimer& timer) override;
	virtual void Draw(const GameTimer& timer) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

	void OnKeyboardInput(const GameTimer& timer);

	void AnimateMaterials(const GameTimer& timer);

	void UpdateCamera(const GameTimer& timer);
	void UpdateObjectCBs(const GameTimer& timer);
	void UpdateMaterialCBs(const GameTimer& timer);
	void UpdateMainPassCB(const GameTimer& timer);
	void UpdateWavesGPU(const GameTimer& timer);

	void LoadTextures();
	void BuildBoxGeometry();
	void BuildLandGeometry();
	void BuildWavesGeometry();
	void BuildMaterials();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Renderable*>& renderables);
	void DrawFullScreenQuad(ID3D12GraphicsCommandList* cmdList);
	void BuildFrameResources();

	void BuildDescriptorHeaps();
	void BuildRootSignature();
	void BuildWavesRootSignature();
	void BuildPostProcessRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSOs();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	float GetHillsHeight(float x, float z) const;
	XMFLOAT3 GetHillsNormal(float x, float z) const;

private:
	// List of all the render items.
	std::vector<std::unique_ptr<Renderable>> m_AllRenderables;

	// Render items divided by PSO.
	std::vector<Renderable*> m_RenderableLayer[(int)RenderLayer::Count];

	// Frame Resources.
	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
	FrameResource* m_CurrFrameResource = nullptr;
	int m_CurrFrameResourceIndex = 0;

	// Descriptor Heaps
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvSrvUavDescriptorHeap = nullptr;

	// Root Signatures.
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_WavesRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_PostProcessRootSignature = nullptr;

	// Shaders and InputLayout.
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_Shaders;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// Pipeline State Objects (PSOs).
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	// Textures
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_Textures;

	// Geometry (vertex/index buffers).
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;

	// Materials
	std::unordered_map<std::string, std::unique_ptr<Material>> m_Materials;

	// Waves
	std::unique_ptr<GpuWaves> m_Waves;

	// Sobel filter
	std::unique_ptr<SobelFilter> m_SobelFilter = nullptr;

	// Offscreen render target
	std::unique_ptr<RenderTarget> m_OffscreenRT = nullptr;

	// Pass constant buffer.
	PassConstants m_MainPassCB;

	// PSO state (solid/wireframe).
	bool m_IsWireframe = false;

	// Demo related members.
	XMFLOAT3   m_EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 m_View  = MathHelper::Identity4x4();
	XMFLOAT4X4 m_Proj  = MathHelper::Identity4x4();

	float m_Theta = 1.5f * XM_PI;
	float m_Phi = 0.1f * XM_PIDIV2;
	float m_Radius = 50.0f;

	POINT m_InitialMousePos;
	POINT m_LastMousePos;
};