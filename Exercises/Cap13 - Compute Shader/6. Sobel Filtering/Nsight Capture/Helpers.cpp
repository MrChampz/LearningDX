//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: Helpers.cpp
// 
//-----------------------------------------------------------------------------

#include <d3d9.h>
#include <d3d12.h>
#include <atlbase.h>
#include <vector>
#include <dxgi1_4.h>

#if !defined(_MSC_VER) || _MSC_VER >= 1700
#include <mutex>
#endif
#include <set>
#include <sstream>

#include "Helpers.h"
#include "Resources.h"
#include "ReadOnlyDatabase.h"

#if !defined(_WIN64) && !defined(__x86_64__) && !defined(__ppc64__) && !defined(__LP64__)
#error Application originally targeted 64-bit; compiling now as 32-bit
#endif

// Tell the driver to use the high performance GPU
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
};

using namespace Serialization;

union HexFloat
{
    float f;
    uint32_t h;
};

union HexDouble
{
    double d;
    uint64_t h;
};

float HexToFloat(uint32_t h)
{
    HexFloat v;
    v.h = h;
    return v.f;
}

double HexToDouble(uint64_t h)
{
    HexDouble v;
    v.h = h;
    return v.d;
}

float NAN_F()
{
    return HexToFloat(0xffc00000);
}

float INF_F()
{
    return HexToFloat(0x7f800000);
}

double NAN_D()
{
    return HexToDouble(0xfff8000000000000);
}

double INF_D()
{
    return HexToDouble(0x7ff0000000000000);
}

//-----------------------------------------------------------------------------
// ReportErrorWithMessage
//-----------------------------------------------------------------------------
void ReportErrorWithMessage(const char* message, const char* file, int line)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    std::stringstream stream;
    stream << message << " at " << file << "(" << line << ")";
    NV_MESSAGE(stream.str().c_str());
}

//-----------------------------------------------------------------------------
// ThrowErrorWithMessage
//-----------------------------------------------------------------------------
void ThrowErrorWithMessage(const char* message, const char* file, int line)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    std::stringstream stream;
    stream << message << " at " << file << "(" << line << ")";
    throw std::runtime_error(stream.str());
}

//-----------------------------------------------------------------------------
// CheckResult
//-----------------------------------------------------------------------------
void CheckResult(HRESULT result, const char* file, int line)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    if (FAILED(result))
    {
        BEGIN_DATA_SCOPE();

        std::stringstream stream;
        stream << "Error 0x" << std::hex << result;
        ThrowErrorWithMessage(stream.str().c_str(), file, line);
    }
}

//-----------------------------------------------------------------------------
// CheckPresentResult
//-----------------------------------------------------------------------------
void CheckPresentResult(HRESULT result, IDXGISwapChain* pSwapChain, const char* file, int line)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    if (FAILED(result))
    {
        BEGIN_DATA_SCOPE();

        DXGI_SWAP_CHAIN_DESC tempDesc;
        pSwapChain->GetDesc(&tempDesc);
        if (tempDesc.SwapEffect != DXGI_SWAP_EFFECT_DISCARD && tempDesc.SwapEffect != DXGI_SWAP_EFFECT_SEQUENTIAL)
        {
            BEGIN_DATA_SCOPE();

            std::stringstream stream;
            stream << "Present Failed with error 0x" << std::hex << result << std::endl << std::endl;
            stream << "Try changing the swap chain swap effect to DXGI_SWAP_EFFECT_DISCARD or DXGI_SWAP_EFFECT_SEQUENTIAL to see if it solves the problem." << std::endl << std::endl;
            ThrowErrorWithMessage(stream.str().c_str(), file, line);
        }
    }
}

//-----------------------------------------------------------------------------
// GetDatabase
//-----------------------------------------------------------------------------
Serialization::ReadOnlyDatabase& GetDatabase()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // Set max page size: 64MB
    const static uint64_t MAX_PAGE_SIZE = 1 << 26;

    // Set max resident page count
    const static size_t MAX_RESIDENT_PAGES = 16;
    static ReadOnlyDatabase s_database("data.bin", MAX_PAGE_SIZE, MAX_RESIDENT_PAGES);
    return s_database;
}

//-----------------------------------------------------------------------------
// FreeCachedMemory
//-----------------------------------------------------------------------------
void FreeCachedMemory()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    GetDatabase().FreeCachedMemory();
}

const GUID NVNameHelper::WKPDID_D3DDebugObjectName = { 0x429b8c22, 0x9188, 0x4b0c, { 0x87, 0x42, 0xac, 0xb0, 0xbf, 0x85, 0xc2, 0x00 } };

//-----------------------------------------------------------------------------
// NVNameHelper::NVSetObjectName
//-----------------------------------------------------------------------------
void NVNameHelper::NVSetObjectName(void* pObj, const char* pName)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // NVSetObjectName does not support this type
}

//-----------------------------------------------------------------------------
// ComputeDXGIResourceSizes
//-----------------------------------------------------------------------------
void ComputeDXGIResourceSizes(DXGI_FORMAT format, unsigned int& bytesPerBlock, unsigned int& blockWidth, unsigned int& blockHeight)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // One byte per block should be safe
    bytesPerBlock = 0;
    blockWidth = 1;
    blockHeight = 1;

    switch (format)
    {
        default:
            NV_MESSAGE("Unknown DXGI format!");
        case DXGI_FORMAT_UNKNOWN:
            bytesPerBlock = 1;
            break;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
                // 128-bit formats
            bytesPerBlock = 16;
            break;
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
                // 96-bit formats
            bytesPerBlock = 12;
            break;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
                // 64-bit formats
            bytesPerBlock = 8;
            break;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
                // 32-bit formats
            bytesPerBlock = 4;
                break;
            break;
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
                // 16-bit formats
            bytesPerBlock = 2;
                break;
            break;
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
                // 8-bit formats
            bytesPerBlock = 1;
                break;
            break;
        case DXGI_FORMAT_R1_UNORM:
                // 1-bit format?  Treat this like a compressed format
                // with a block width of 8 and 1 byte per block
            bytesPerBlock = 1;
            blockWidth = 8;
                break;
            break;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
                // Funky 32-bit formats
            bytesPerBlock = 4;
                break;
            break;
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
                // These are listed as 32-bit formats, but they aren't.  The documentation
                // says textures with these formats must have a width that is a multiple of 2.
                // It seems that these formats alternate R8G8 for one texel and B8G8 for the
                // next (or G8R8 then G8B8).  Thus, there is a block width of 2.
            bytesPerBlock = 4;
            blockWidth = 2;
                break;
            break;
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
                // 8 bytes per 4x4 texel block compressed formats
            bytesPerBlock = 8;
            blockWidth = 4;
            blockHeight = 4;
                break;
            break;
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
                // 16 bytes per 4x4 texel block
            bytesPerBlock = 16;
            blockWidth = 4;
            blockHeight = 4;
                break;
            break;
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
                // More 16-bit formats
            bytesPerBlock = 2;
            break;
    }
}

//-----------------------------------------------------------------------------
// D3D12OnPresent
//-----------------------------------------------------------------------------
void D3D12OnPresent(IDXGISwapChain3* pSwapChain, ID3D12Resource* pOffscreenSurface, ID3D12Device* pDevice, ID3D12CommandQueue* pQueue, std::vector<CComPtr<ID3D12GraphicsCommandList>>& commandListVector, std::vector<CComPtr<ID3D12CommandAllocator>>& allocatorVector)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    if (commandListVector.empty())
    {
        BEGIN_DATA_SCOPE();

        DXGI_SWAP_CHAIN_DESC DXGI_SWAP_CHAIN_DESC_temp_1;
        pSwapChain->GetDesc(&DXGI_SWAP_CHAIN_DESC_temp_1);
        commandListVector.resize(DXGI_SWAP_CHAIN_DESC_temp_1.BufferCount, NULL);
        allocatorVector.resize(DXGI_SWAP_CHAIN_DESC_temp_1.BufferCount, NULL);
    }

    UINT UINT_temp_1 = pSwapChain->GetCurrentBackBufferIndex();
    if (commandListVector[UINT_temp_1] == NULL)
    {
        BEGIN_DATA_SCOPE();

        result = pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&allocatorVector[UINT_temp_1]);
NV_CHECK_RESULT(result);

        result = pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocatorVector[UINT_temp_1], NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&commandListVector[UINT_temp_1]);
NV_CHECK_RESULT(result);

        CComPtr<ID3D12Resource> CComPtr_of_ID3D12Resource_temp_1 = NULL;
        result = pSwapChain->GetBuffer(UINT_temp_1, __uuidof(ID3D12Resource), ((void**)&CComPtr_of_ID3D12Resource_temp_1));
NV_CHECK_RESULT(result);

        D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_1[2] = {{D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, {CComPtr_of_ID3D12Resource_temp_1, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST}}, {D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, {pOffscreenSurface, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_SOURCE}}};
        commandListVector[UINT_temp_1]->ResourceBarrier(2, D3D12_RESOURCE_BARRIER_temp_1);
        commandListVector[UINT_temp_1]->CopyResource(CComPtr_of_ID3D12Resource_temp_1, pOffscreenSurface);
        D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_2[2] = {{D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, {CComPtr_of_ID3D12Resource_temp_1, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT}}, {D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE, {pOffscreenSurface, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT}}};
        commandListVector[UINT_temp_1]->ResourceBarrier(2, D3D12_RESOURCE_BARRIER_temp_2);
        commandListVector[UINT_temp_1]->Close();
    }

    ID3D12CommandList* pID3D12CommandList_temp_2 = commandListVector[UINT_temp_1];
    pQueue->ExecuteCommandLists(1, &pID3D12CommandList_temp_2);
}

//-----------------------------------------------------------------------------
// D3D12Finish
//-----------------------------------------------------------------------------
void D3D12Finish(ID3D12CommandQueue* pQueue, ID3D12Fence* pFence, UINT64& value)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    ++value;
    pQueue->Signal(pFence, value);
    while (pFence->GetCompletedValue() < value)
    {
        BEGIN_DATA_SCOPE();

        ::Sleep(0);
    }
}

//-----------------------------------------------------------------------------
// D3D12InitResourceData
//-----------------------------------------------------------------------------
void D3D12InitResourceData(ID3D12Device* pDevice, ID3D12CommandQueue* pQueue, ID3D12GraphicsCommandList* pCommandList, ID3D12Resource* pResource, ID3D12Resource*& pUploadResource, UINT subresourceCount, const Serialization::DATABASE_HANDLE* pDatabaseHandles, UINT resourceBarrierCount, const D3D12_RESOURCE_BARRIER* pResourceBarriers)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    D3D12_RESOURCE_DESC D3D12_RESOURCE_DESC_temp_1 = pResource->GetDesc();
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> std_vector_of_D3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1;
    std_vector_of_D3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1.resize(subresourceCount);
    UINT64 UINT64_temp_1;
    pDevice->GetCopyableFootprints(&D3D12_RESOURCE_DESC_temp_1, 0, subresourceCount, 0, &std_vector_of_D3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1[0], NULL, NULL, &UINT64_temp_1);
    if (!pUploadResource)
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_DESC D3D12_RESOURCE_DESC_temp_2 = {D3D12_RESOURCE_DIMENSION_BUFFER, 0ull, 0ull, 1u, 1, 1, DXGI_FORMAT_UNKNOWN, {1u, 0u}, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE};
        D3D12_RESOURCE_DESC_temp_2.Width = UINT64_temp_1;
        static D3D12_HEAP_PROPERTIES D3D12_HEAP_PROPERTIES_temp_1 = {D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1u, 1u};
        result = pDevice->CreateCommittedResource(&D3D12_HEAP_PROPERTIES_temp_1, D3D12_HEAP_FLAG_NONE, &D3D12_RESOURCE_DESC_temp_2, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(ID3D12Resource), (void**)&pUploadResource);
        NV_CHECK_RESULT(result);
        uint8_t* puint8_t_temp_1 = NULL;

        result = pUploadResource->Map(0, NULL, ((void**)&puint8_t_temp_1));
        NV_CHECK_RESULT(result);

        for (UINT subresource = 0; subresource < subresourceCount; ++subresource)
        {
            BEGIN_DATA_SCOPE();

            D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pD3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1 = &std_vector_of_D3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1[subresource];

            UINT bytesPerBlock;
            UINT blockWidth;
            UINT blockHeight;
            ComputeDXGIResourceSizes(pD3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1->Footprint.Format, bytesPerBlock, blockWidth, blockHeight);

            UINT UINT_temp_2 = subresource % D3D12_RESOURCE_DESC_temp_1.MipLevels;
            UINT64 UINT64_temp_2 = (max(blockWidth, D3D12_RESOURCE_DESC_temp_1.Width >> UINT_temp_2) + blockWidth - 1) / blockWidth;
            UINT UINT_temp_3 = (max(blockHeight, D3D12_RESOURCE_DESC_temp_1.Height >> UINT_temp_2) + blockHeight - 1) / blockHeight;
            UINT UINT_temp_4 = D3D12_RESOURCE_DESC_temp_1.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? max(1u, D3D12_RESOURCE_DESC_temp_1.DepthOrArraySize >> UINT_temp_2) : 1u;
            UINT64 UINT64_temp_3 = UINT64_temp_2 * bytesPerBlock;
            uint8_t* puint8_t_temp_2 = NV_GET_RESOURCE(uint8_t*, pDatabaseHandles[subresource]);
            for (UINT z = 0; z < UINT_temp_4; ++z)
            {
                BEGIN_DATA_SCOPE();

                for (UINT y = 0; y < UINT_temp_3; ++y)
                {
                    BEGIN_DATA_SCOPE();
                    memcpy(puint8_t_temp_1 + pD3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1->Offset + (y + (z * UINT_temp_3)) * pD3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1->Footprint.RowPitch, puint8_t_temp_2 + (y + (z * UINT_temp_3)) * UINT64_temp_3, UINT64_temp_3);
                }
            }
        }

        pUploadResource->Unmap(0, NULL);
    }

    if (resourceBarrierCount)
    {
        BEGIN_DATA_SCOPE();

        pCommandList->ResourceBarrier(resourceBarrierCount, pResourceBarriers);
    }

    if (D3D12_RESOURCE_DESC_temp_1.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
    {
        BEGIN_DATA_SCOPE();

        pCommandList->CopyResource(pResource, pUploadResource);
    }
    else
    {
        BEGIN_DATA_SCOPE();

        for (UINT subresource = 0; subresource < subresourceCount; ++subresource)
        {
            BEGIN_DATA_SCOPE();

            D3D12_TEXTURE_COPY_LOCATION D3D12_TEXTURE_COPY_LOCATION_temp_1 = {pResource, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX };
            D3D12_TEXTURE_COPY_LOCATION_temp_1.SubresourceIndex = subresource;
            D3D12_TEXTURE_COPY_LOCATION D3D12_TEXTURE_COPY_LOCATION_temp_2 = {pUploadResource, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, std_vector_of_D3D12_PLACED_SUBRESOURCE_FOOTPRINT_temp_1[subresource]};
            pCommandList->CopyTextureRegion(&D3D12_TEXTURE_COPY_LOCATION_temp_1, 0, 0, 0, &D3D12_TEXTURE_COPY_LOCATION_temp_2, NULL);
        }
    }

    if (resourceBarrierCount)
    {
        BEGIN_DATA_SCOPE();

        std::vector<D3D12_RESOURCE_BARRIER> reverseBarriers(pResourceBarriers, pResourceBarriers + resourceBarrierCount);
        for (size_t i = 0; i < resourceBarrierCount; ++i)
        {
            BEGIN_DATA_SCOPE();

            std::swap(reverseBarriers[i].Transition.StateBefore, reverseBarriers[i].Transition.StateAfter);
        }

        pCommandList->ResourceBarrier(resourceBarrierCount, &reverseBarriers[0]);
    }
}

//-----------------------------------------------------------------------------
// D3D12TransitionBarrier
//-----------------------------------------------------------------------------
D3D12_RESOURCE_BARRIER D3D12TransitionBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, UINT subresource)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_3;
    D3D12_RESOURCE_BARRIER_temp_3.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    D3D12_RESOURCE_BARRIER_temp_3.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    D3D12_RESOURCE_BARRIER_temp_3.Transition.pResource = pResource;
    D3D12_RESOURCE_BARRIER_temp_3.Transition.Subresource = subresource;
    D3D12_RESOURCE_BARRIER_temp_3.Transition.StateBefore = beforeState;
    D3D12_RESOURCE_BARRIER_temp_3.Transition.StateAfter = afterState;
    return D3D12_RESOURCE_BARRIER_temp_3;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVBuffer
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVBuffer(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT64 firstElement, UINT numElements, UINT stuctureByteStride, D3D12_BUFFER_SRV_FLAGS flags)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_BUFFER, shader4ComponentMapping};
    desc.Buffer.FirstElement = firstElement;
    desc.Buffer.NumElements = numElements;
    desc.Buffer.StructureByteStride = stuctureByteStride;
    desc.Buffer.Flags = flags;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTexture1D
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture1D(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, FLOAT resourceMinLODClamp)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURE1D, shader4ComponentMapping};
    desc.Texture1D.MostDetailedMip = mostDetailedMip;
    desc.Texture1D.MipLevels = mipLevels;
    desc.Texture1D.ResourceMinLODClamp = resourceMinLODClamp;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTexture1DArray
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture1DArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT firstArraySlice, UINT arraySize, FLOAT resourceMinLODClamp)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURE1DARRAY, shader4ComponentMapping};
    desc.Texture1DArray.MostDetailedMip = mostDetailedMip;
    desc.Texture1DArray.MipLevels = mipLevels;
    desc.Texture1DArray.FirstArraySlice = firstArraySlice;
    desc.Texture1DArray.ArraySize = arraySize;
    desc.Texture1DArray.ResourceMinLODClamp = resourceMinLODClamp;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTexture2D
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2D(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT planeSlice, FLOAT resourceMinLODClamp)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURE2D, shader4ComponentMapping};
    desc.Texture2D.MostDetailedMip = mostDetailedMip;
    desc.Texture2D.MipLevels = mipLevels;
    desc.Texture2D.PlaneSlice = planeSlice;
    desc.Texture2D.ResourceMinLODClamp = resourceMinLODClamp;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTexture2DArray
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2DArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT firstArraySlice, UINT arraySize, UINT planeSlice, FLOAT resourceMinLODClamp)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURE2DARRAY, shader4ComponentMapping};
    desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
    desc.Texture2DArray.MipLevels = mipLevels;
    desc.Texture2DArray.FirstArraySlice = firstArraySlice;
    desc.Texture2DArray.ArraySize = arraySize;
    desc.Texture2DArray.PlaneSlice = planeSlice;
    desc.Texture2DArray.ResourceMinLODClamp = resourceMinLODClamp;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTexture2DMS
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2DMS(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURE2DMS, shader4ComponentMapping};
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTexture2DMSArray
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture2DMSArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT firstArraySlice, UINT arraySize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY, shader4ComponentMapping};
    desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
    desc.Texture2DMSArray.ArraySize = arraySize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTexture3D
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTexture3D(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, FLOAT resourceMinLODClamp)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURE3D, shader4ComponentMapping};
    desc.Texture3D.MostDetailedMip = mostDetailedMip;
    desc.Texture3D.MipLevels = mipLevels;
    desc.Texture3D.ResourceMinLODClamp = resourceMinLODClamp;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTextureCube
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTextureCube(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, FLOAT resourceMinLODClamp)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURECUBE, shader4ComponentMapping};
    desc.TextureCube.MostDetailedMip = mostDetailedMip;
    desc.TextureCube.MipLevels = mipLevels;
    desc.TextureCube.ResourceMinLODClamp = resourceMinLODClamp;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSRVTextureCubeArray
//-----------------------------------------------------------------------------
D3D12_SHADER_RESOURCE_VIEW_DESC* D3D12InitSRVTextureCubeArray(D3D12_SHADER_RESOURCE_VIEW_DESC& desc, DXGI_FORMAT format, UINT shader4ComponentMapping, UINT mostDetailedMip, UINT mipLevels, UINT first2DArrayFace, UINT numCubes, FLOAT resourceMinLODClamp)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_SRV_DIMENSION_TEXTURECUBEARRAY, shader4ComponentMapping};
    desc.TextureCubeArray.MostDetailedMip = mostDetailedMip;
    desc.TextureCubeArray.MipLevels = mipLevels;
    desc.TextureCubeArray.First2DArrayFace = first2DArrayFace;
    desc.TextureCubeArray.NumCubes = numCubes;
    desc.TextureCubeArray.ResourceMinLODClamp = resourceMinLODClamp;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitUAVBuffer
//-----------------------------------------------------------------------------
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVBuffer(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT64 firstElement, UINT numElements, UINT stuctureByteStride, UINT64 counterOffsetInBytes, D3D12_BUFFER_UAV_FLAGS flags)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_UAV_DIMENSION_BUFFER};
    desc.Buffer.FirstElement = firstElement;
    desc.Buffer.NumElements = numElements;
    desc.Buffer.StructureByteStride = stuctureByteStride;
    desc.Buffer.CounterOffsetInBytes = counterOffsetInBytes;
    desc.Buffer.Flags = flags;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitUAVTexture1D
//-----------------------------------------------------------------------------
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture1D(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_UAV_DIMENSION_TEXTURE1D};
    desc.Texture1D.MipSlice = mipSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitUAVTexture1DArray
//-----------------------------------------------------------------------------
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture1DArray(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_UAV_DIMENSION_TEXTURE1DARRAY};
    desc.Texture1DArray.MipSlice = mipSlice;
    desc.Texture1DArray.FirstArraySlice = firstArraySlice;
    desc.Texture1DArray.ArraySize = arraySize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitUAVTexture2D
//-----------------------------------------------------------------------------
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture2D(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT planeSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_UAV_DIMENSION_TEXTURE2D};
    desc.Texture2D.MipSlice = mipSlice;
    desc.Texture2D.PlaneSlice = planeSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitUAVTexture2DArray
//-----------------------------------------------------------------------------
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture2DArray(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize, UINT planeSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_UAV_DIMENSION_TEXTURE2DARRAY};
    desc.Texture2DArray.MipSlice = mipSlice;
    desc.Texture2DArray.FirstArraySlice = firstArraySlice;
    desc.Texture2DArray.ArraySize = arraySize;
    desc.Texture2DArray.PlaneSlice = planeSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitUAVTexture3D
//-----------------------------------------------------------------------------
D3D12_UNORDERED_ACCESS_VIEW_DESC* D3D12InitUAVTexture3D(D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice, UINT wSize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_UAV_DIMENSION_TEXTURE3D};
    desc.Texture3D.MipSlice = mipSlice;
    desc.Texture3D.FirstWSlice = firstWSlice;
    desc.Texture3D.WSize = wSize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVBuffer
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVBuffer(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT64 firstElement, UINT numElements)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_BUFFER};
    desc.Buffer.FirstElement = firstElement;
    desc.Buffer.NumElements = numElements;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVTexture1D
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture1D(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_TEXTURE1D};
    desc.Texture1D.MipSlice = mipSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVTexture1DArray
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture1DArray(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_TEXTURE1DARRAY};
    desc.Texture1DArray.MipSlice = mipSlice;
    desc.Texture1DArray.FirstArraySlice = firstArraySlice;
    desc.Texture1DArray.ArraySize = arraySize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVTexture2D
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2D(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT planeSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_TEXTURE2D};
    desc.Texture2D.MipSlice = mipSlice;
    desc.Texture2D.PlaneSlice = planeSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVTexture2DArray
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2DArray(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize, UINT planeSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_TEXTURE2DARRAY};
    desc.Texture2DArray.MipSlice = mipSlice;
    desc.Texture2DArray.FirstArraySlice = firstArraySlice;
    desc.Texture2DArray.ArraySize = arraySize;
    desc.Texture2DArray.PlaneSlice = planeSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVTexture2DMS
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2DMS(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_TEXTURE2DMS};
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVTexture2DMSArray
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture2DMSArray(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT firstArraySlice, UINT arraySize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY};
    desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
    desc.Texture2DMSArray.ArraySize = arraySize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitRTVTexture3D
//-----------------------------------------------------------------------------
D3D12_RENDER_TARGET_VIEW_DESC* D3D12InitRTVTexture3D(D3D12_RENDER_TARGET_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice, UINT wSize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_RTV_DIMENSION_TEXTURE3D};
    desc.Texture3D.MipSlice = mipSlice;
    desc.Texture3D.FirstWSlice = firstWSlice;
    desc.Texture3D.WSize = wSize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitDSVTexture1D
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture1D(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_DSV_DIMENSION_TEXTURE1D};
    desc.Texture1D.MipSlice = mipSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitDSVTexture1DArray
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture1DArray(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_DSV_DIMENSION_TEXTURE1DARRAY};
    desc.Texture1DArray.MipSlice = mipSlice;
    desc.Texture1DArray.FirstArraySlice = firstArraySlice;
    desc.Texture1DArray.ArraySize = arraySize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitDSVTexture2D
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2D(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_DSV_DIMENSION_TEXTURE2D};
    desc.Texture2D.MipSlice = mipSlice;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitDSVTexture2DArray
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2DArray(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_DSV_DIMENSION_TEXTURE2DARRAY};
    desc.Texture2DArray.MipSlice = mipSlice;
    desc.Texture2DArray.FirstArraySlice = firstArraySlice;
    desc.Texture2DArray.ArraySize = arraySize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitDSVTexture2DMS
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2DMS(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_DSV_DIMENSION_TEXTURE2DMS};
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitDSVTexture2DMSArray
//-----------------------------------------------------------------------------
D3D12_DEPTH_STENCIL_VIEW_DESC* D3D12InitDSVTexture2DMSArray(D3D12_DEPTH_STENCIL_VIEW_DESC& desc, DXGI_FORMAT format, UINT firstArraySlice, UINT arraySize)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {format, D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY};
    desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
    desc.Texture2DMSArray.ArraySize = arraySize;
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitCBVDesc
//-----------------------------------------------------------------------------
D3D12_CONSTANT_BUFFER_VIEW_DESC* D3D12InitCBVDesc(D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, UINT sizeInBytes)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {bufferLocation, sizeInBytes};
    return &desc;
}

//-----------------------------------------------------------------------------
// D3D12InitSamplerDesc
//-----------------------------------------------------------------------------
D3D12_SAMPLER_DESC* D3D12InitSamplerDesc(D3D12_SAMPLER_DESC& desc, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressU, D3D12_TEXTURE_ADDRESS_MODE addressV, D3D12_TEXTURE_ADDRESS_MODE addressW, FLOAT mipLODBias, UINT maxAnisotropy, D3D12_COMPARISON_FUNC comparisonFunc, FLOAT borderColor[4], FLOAT minLOD, FLOAT maxLOD)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    desc = {filter, addressU, addressV, addressW, mipLODBias, maxAnisotropy, comparisonFunc, {borderColor[0], borderColor[1], borderColor[2], borderColor[3]}, minLOD, maxLOD};
    return &desc;
}

//-----------------------------------------------------------------------------
// ReplayerWndProc
//-----------------------------------------------------------------------------
static LRESULT CALLBACK ReplayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // Handle messages we care about
    switch (uMsg)
    {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                BEGIN_DATA_SCOPE();

                PostQuitMessage(0);
                return 0;
            }

            break;
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
            break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// TrySetProcessDpiAwareness
//-----------------------------------------------------------------------------
static HRESULT TrySetProcessDpiAwareness(PROCESS_DPI_AWARENESS value)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // Load up DLL responsible for DPI management (requires Win8.1 and above)
    HMODULE hShcore = LoadLibraryA("Shcore.dll");
    if (hShcore == NULL)
    {
        BEGIN_DATA_SCOPE();

        return S_OK;
    }

    // Find up SetProcessDpiAwareness
    typedef HRESULT(STDAPICALLTYPE * PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS value);
    PFN_SetProcessDpiAwareness fnSetProcessDpiAwareness = reinterpret_cast<PFN_SetProcessDpiAwareness>(GetProcAddress(hShcore, "SetProcessDpiAwareness"));
    if (fnSetProcessDpiAwareness == NULL)
    {
        BEGIN_DATA_SCOPE();

        return S_OK;
    }

    // Set value if the function exists
    return fnSetProcessDpiAwareness(value);
}

//-----------------------------------------------------------------------------
// RegisterReplayerWindowClass
//-----------------------------------------------------------------------------
BOOL RegisterReplayerWindowClass(HINSTANCE hInstance, UINT style)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // Populate the struct
    WNDCLASS wc;
    wc.style = style;
    wc.lpfnWndProc = (WNDPROC)ReplayerWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = REPLAYER_WINDOW_MENU_NAME;
    wc.lpszClassName = REPLAYER_WINDOW_CLASS_NAME;
    BOOL ret = RegisterClass(&wc);
    if (!ret && GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
    {
        BEGIN_DATA_SCOPE();

        return TRUE;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// CreateReplayerWindow
//-----------------------------------------------------------------------------
HWND CreateReplayerWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, uint32_t x, uint32_t y, uint32_t Width, uint32_t Height, DWORD Style, DWORD ExtendedStyle, PROCESS_DPI_AWARENESS DpiAwareness)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // Respect captured process's DPI awareness
    TrySetProcessDpiAwareness(DpiAwareness);

    // Adjust the window rect based on style
    RECT WindowRect;
    WindowRect.left = x;
    WindowRect.right = x + Width;
    WindowRect.top = y;
    WindowRect.bottom = y + Height;
    AdjustWindowRectEx(&WindowRect, Style, NULL, ExtendedStyle);
    Width = WindowRect.right - WindowRect.left;
    Height = WindowRect.bottom - WindowRect.top;

    // Create the window.
    return CreateWindowEx(ExtendedStyle, REPLAYER_WINDOW_CLASS_NAME, REPLAYER_WINDOW_TITLE_NAME, Style, x, y, Width, Height, (HWND)NULL, (HMENU)NULL, hInstance, (LPVOID)NULL);
}

//-----------------------------------------------------------------------------
// Common Resources
//-----------------------------------------------------------------------------
HRESULT result;
HINSTANCE currentInstance = NULL;
bool hasFrameReset = true;
bool forceDebug = false;
double array_of_0s[256] = { 0 };
char errorBuf[512];
