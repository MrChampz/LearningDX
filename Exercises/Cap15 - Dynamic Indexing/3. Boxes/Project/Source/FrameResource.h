#pragma once

#include <D3DUtil.h>
#include <MathHelper.h>
#include <UploadBuffer.h>

struct ObjectConstants
{
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT	   MaterialIndex;
	UINT	   _ObjPad0;
	UINT	   _ObjPad1;
	UINT	   _ObjPad2;
};

struct PassConstants
{
	XMFLOAT4X4 View = MathHelper::Identity4x4();
	XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
	XMFLOAT4X4 InvView = MathHelper::Identity4x4();
	XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	XMFLOAT3   EyePosW = { 0.0f, 0.0f, 0.0f };
	float      _PassPad1 = 0.0f;
	XMFLOAT2   RenderTargetSize = { 0.0f, 0.0f };
	XMFLOAT2   InvRenderTargetSize = { 0.0f, 0.0f };
	float      NearZ = 0.0f;
	float      FarZ = 0.0f;
	float      TotalTime = 0.0f;
	float      DeltaTime = 0.0f;

	XMFLOAT4   AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHTS] are point lights;
	// indices [NUM_DIR_LIGHTS + NUM_POINT_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHT + NUM_SPOT_LIGHTS]
	// are spot lights for a maximum of MaxLights per object.
	Light      Lights[MaxLights];
};

struct MaterialData
{
	XMFLOAT4   DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT3   FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float      Roughness = 64.0f;

	// Used in texture mapping.
	XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	UINT     DiffuseMapIndex;
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.
struct FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

public:
	// We cannot reset the allocator until the GPU is done processing the commands.
	// So each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	// We cannot update a cbuffer until the GPU is done processing the commands
	// that reference it. So each frame needs their own cbuffers.
	std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

	// Material structured buffer.
	std::unique_ptr<UploadBuffer<MaterialData>> MaterialBuffer = nullptr;

	// Fence value to mark commands up to this fence point. This lets us
	// check if these frame resources are still in use by the GPU.
	UINT64 Fence = 0;
};