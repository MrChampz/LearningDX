//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: Helpers.h
// 
//-----------------------------------------------------------------------------

#pragma once

#include <d3d9.h>
#include <d3d12.h>
#include <atlbase.h>
#include <vector>
#include <dxgi1_4.h>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <map>
#include <set>

#include "ReadOnlyDatabase.h"

//-----------------------------------------------------------------------------
// High DPI Support
//-----------------------------------------------------------------------------
#if (NTDDI_VERSION >= NTDDI_WINBLUE)
#pragma comment(lib, "Shcore.lib")
#include <ShellScalingApi.h>
#else

typedef enum PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
#endif

//-----------------------------------------------------------------------------
// Common Resources
//-----------------------------------------------------------------------------
extern HRESULT result;
extern HINSTANCE currentInstance;
extern bool hasFrameReset;
extern bool forceDebug;
extern double array_of_0s[256];
extern char errorBuf[512];

//-----------------------------------------------------------------------------
// Window Specification
//-----------------------------------------------------------------------------
const wchar_t REPLAYER_WINDOW_CLASS_NAME[] = L"NV Replayer WndClass";
const wchar_t REPLAYER_WINDOW_TITLE_NAME[] = L"Nvda.Replayer";
const wchar_t REPLAYER_WINDOW_MENU_NAME[] = L"";

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif

#define NV_SAFE_RELEASE(_pObject) if (_pObject) { _pObject->Release(); _pObject = NULL; }
#define NV_SAFE_DESTROY(_pObject) if (_pObject) { _pObject->Destroy(); _pObject = NULL; }

extern float HexToFloat(uint32_t h);
extern double HexToDouble(uint64_t h);
extern float NAN_F();
extern float INF_F();
extern double NAN_D();
extern double INF_D();

extern void ReportErrorWithMessage(const char* message, const char* file, int line);
extern void ThrowErrorWithMessage(const char* message, const char* file, int line);
#define NV_THROW_IF(_Condition, _Message) if (_Condition) { ThrowErrorWithMessage(_Message, __FILE__, __LINE__); }
#define NV_ASSERT(_Condition) assert(_Condition)
#ifdef _WIN32
#define NV_MESSAGE(_Message) ::OutputDebugStringA(_Message)
#else
#define NV_MESSAGE(_Message) printf("%s", _Message)
#endif

extern void CheckResult(HRESULT result, const char* file, int line);
#define NV_CHECK_RESULT(_result) CheckResult(_result, __FILE__, __LINE__)

void CheckPresentResult(HRESULT result, IDXGISwapChain* pSwapChain, const char* file, int line);
#define NV_CHECK_PRESENT_RESULT(_result, _pSwapChain) CheckPresentResult(_result, _pSwapChain, __FILE__, __LINE__)

Serialization::ReadOnlyDatabase& GetDatabase();
#if defined(__arm__)
template<typename T, typename DataScopeTrackerType>
T GetResources(DataScopeTrackerType& dataScopeTracker, Serialization::DATABASE_HANDLE handle)
{
    std::shared_ptr<Serialization::BlobProxyBase> spBlobProxy = GetDatabase().ReadShared<T>(handle);
    dataScopeTracker.AddBlobProxyToCurrentDataScope(spBlobProxy);
    return (std::static_pointer_cast<Serialization::BlobProxy<T> >(spBlobProxy))->Get();
}
#define NV_GET_RESOURCE(T, handle) GetResources<T>(dataScopeTracker, handle)
#else
#define NV_GET_RESOURCE(T, handle) GetDatabase().Read<T>(handle).Get()
#endif //defined(__arm__)
extern void FreeCachedMemory();

#define NV_SET_OBJECT_NAME(X, NAME) NVNameHelper::NVSetObjectName(X, NAME)
struct NVNameHelper
{
    static const GUID WKPDID_D3DDebugObjectName;
    static void NVSetObjectName(void* pObj, const char* pName);
};

//-----------------------------------------------------------------------------
// OffsetCPUDescriptor
//-----------------------------------------------------------------------------
inline D3D12_CPU_DESCRIPTOR_HANDLE OffsetCPUDescriptor(UINT descriptorOffset, ID3D12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    D3D12_CPU_DESCRIPTOR_HANDLE start = pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    return { start.ptr + descriptorOffset * pDevice->GetDescriptorHandleIncrementSize(pDescriptorHeap->GetDesc().Type) };
}

//-----------------------------------------------------------------------------
// OffsetGPUDescriptor
//-----------------------------------------------------------------------------
inline D3D12_GPU_DESCRIPTOR_HANDLE OffsetGPUDescriptor(UINT descriptorOffset, ID3D12Device* pDevice, ID3D12DescriptorHeap* pDescriptorHeap)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    D3D12_GPU_DESCRIPTOR_HANDLE start = pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    return { start.ptr + descriptorOffset * pDevice->GetDescriptorHandleIncrementSize(pDescriptorHeap->GetDesc().Type) };
}

void D3D12OnPresent(IDXGISwapChain3* pSwapChain, ID3D12Resource* pOffscreenSurface, ID3D12Device* pDevice, ID3D12CommandQueue* pQueue, std::vector<CComPtr<ID3D12GraphicsCommandList>>& commandListVector, std::vector<CComPtr<ID3D12CommandAllocator>>& allocatorVector);
void D3D12Finish(ID3D12CommandQueue* pQueue, ID3D12Fence* pFence, UINT64& value);
void D3D12InitResourceData(ID3D12Device* pDevice, ID3D12CommandQueue* pQueue, ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pResource, ID3D12Resource*& pUploadResource, UINT subresourceCount, const Serialization::DATABASE_HANDLE* pDatabaseHandles, UINT resourceBarrierCount, const D3D12_RESOURCE_BARRIER* pResourceBarriers);
D3D12_RESOURCE_BARRIER D3D12TransitionBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT subresource);

// D3D12 SRV helpers
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVBuffer(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT64 firstElement, UINT numElements, UINT stuctureByteStride, D3D12_BUFFER_SRV_FLAGS flags);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture1D(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, FLOAT resourceMinLODClamp);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture1DArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT firstArraySlice, UINT arraySize, FLOAT resourceMinLODClamp);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2D(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT planeSlice, FLOAT resourceMinLODClamp);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2DArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT firstArraySlice, UINT arraySize, UINT planeSlice, FLOAT resourceMinLODClamp);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2DMS(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2DMSArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT firstArraySlice, UINT arraySize);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture3D(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, FLOAT resourceMinLODClamp);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTextureCube(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, FLOAT resourceMinLODClamp);
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTextureCubeArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT first2DArrayFace, UINT numCubes, FLOAT resourceMinLODClamp);

// D3D12 UAV helpers
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVBuffer(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT64 firstElement, UINT numElements, UINT stuctureByteStride, UINT64 counterOffsetInBytes, D3D12_BUFFER_UAV_FLAGS flags);
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture1D(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice);
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture1DArray(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize);
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture2D(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT planeSlice);
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture2DArray(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize, UINT planeSlice);
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture3D(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice, UINT wSize);

// D3D12 RTV helpers
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVBuffer(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT64 firstElement, UINT numElements);
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture1D(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice);
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture1DArray(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize);
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2D(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT planeSlice);
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2DArray(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize, UINT planeSlice);
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2DMS(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format);
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2DMSArray(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT firstArraySlice, UINT arraySize);
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture3D(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice, UINT wSize);

// D3D12 DSV helpers
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture1D(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice);
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture1DArray(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize);
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2D(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice);
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2DArray(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize);
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2DMS(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format);
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2DMSArray(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT firstArraySlice, UINT arraySize);

// D3D12 CBV helpers
D3D12_CONSTANT_BUFFER_VIEW_DESC* D3D12InitCBVDesc(D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, UINT sizeInBytes);

// D3D12 Sampler helpers
D3D12_SAMPLER_DESC* D3D12InitSamplerDesc(D3D12_SAMPLER_DESC& desc, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressU, D3D12_TEXTURE_ADDRESS_MODE addressV, D3D12_TEXTURE_ADDRESS_MODE addressW, FLOAT mipLODBias, UINT maxAnisotropy, D3D12_COMPARISON_FUNC comparisonFunc, FLOAT borderColor[4], FLOAT minLOD, FLOAT maxLOD);

BOOL RegisterReplayerWindowClass(HINSTANCE hInstance, UINT style);
HWND CreateReplayerWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, uint32_t x, uint32_t y, uint32_t Width, uint32_t Height, DWORD Style, DWORD ExtendedStyle, PROCESS_DPI_AWARENESS DpiAwareness);
