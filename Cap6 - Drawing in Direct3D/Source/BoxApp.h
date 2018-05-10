#pragma once

#include <D3DApp.h>
#include <D3DUtil.h>
#include <GameTimer.h>
#include <UploadBuffer.h>
#include <MathHelper.h>

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInst);
	BoxApp(const BoxApp& rhs) = delete;
	BoxApp& operator=(const BoxApp& rhs) = delete;
	~BoxApp();

	virtual bool Initialize() override;

private:
	virtual void OnResize() override;
	virtual void Update(const GameTimer& timer) override;
	virtual void Draw(const GameTimer& timer) override;

	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildPSO();
	void BuildBoxGeometry();

private:
	// Descriptor Heaps
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvHeap = nullptr;

	// Constant buffer
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;

	// Root Signature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;

	// Shaders and InputLayout
	Microsoft::WRL::ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_psByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// Pipeline State Object (PSO)
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PSO = nullptr;

	// Geometry (vertex/index buffers)
	std::unique_ptr<MeshGeometry> m_BoxGeo = nullptr;

	// Demo related members
	XMFLOAT4X4 m_World = MathHelper::Identity4x4();
	XMFLOAT4X4 m_View  = MathHelper::Identity4x4();
	XMFLOAT4X4 m_Proj  = MathHelper::Identity4x4();

	float m_Theta = 1.5f * XM_PI;
	float m_Phi = XM_PIDIV4;
	float m_Radius = 5.0f;

	POINT m_LastMousePos;
};