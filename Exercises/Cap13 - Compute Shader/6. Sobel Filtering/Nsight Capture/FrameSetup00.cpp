//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: FrameSetup00.cpp
// 
//-----------------------------------------------------------------------------

#include <vector>
#include <sstream>

#include <d3d9.h>
#include <d3d12.h>
#include <atlbase.h>
#include <vector>
#include <dxgi1_4.h>

#include "Helpers.h"
#include "Resources.h"

//-----------------------------------------------------------------------------
// SetInitialFrameState00
//-----------------------------------------------------------------------------
void SetInitialFrameState00()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    // Creating offscreen surface for rendering
    {
        BEGIN_DATA_SCOPE();

        static D3D12_HEAP_PROPERTIES D3D12_HEAP_PROPERTIES_temp_2 = {D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1u, 1u};
        static D3D12_RESOURCE_DESC D3D12_RESOURCE_DESC_temp_3 = {D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0ull, 1280ull, 720u, 1, 1, DXGI_FORMAT_R8G8B8A8_TYPELESS, {1u, 0u}, D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET};
        static D3D12_CLEAR_VALUE D3D12_CLEAR_VALUE_temp_1 = {DXGI_FORMAT_R8G8B8A8_UNORM, {0.0f, 0.0f, 0.0f, 0.0f}};
        result = pID3D12Device__uid_3->CreateCommittedResource(&D3D12_HEAP_PROPERTIES_temp_2, D3D12_HEAP_FLAG_NONE, &D3D12_RESOURCE_DESC_temp_3, D3D12_RESOURCE_STATE_RENDER_TARGET, &D3D12_CLEAR_VALUE_temp_1, __uuidof(ID3D12Resource), ((void**)&pIDXGISwapChain_uid_11_offscreen_surface));
        NV_CHECK_RESULT(result);
    }

    result = pID3D12Device__uid_3->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), ((void**)&pID3D12Device__uid_3_initAllocator));
    NV_CHECK_RESULT(result);

    result = pID3D12Device__uid_3->CreateCommandList(0u, D3D12_COMMAND_LIST_TYPE_DIRECT, pID3D12Device__uid_3_initAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), ((void**)&pID3D12Device__uid_3_initCommandList));
    NV_CHECK_RESULT(result);

    result = pID3D12Device__uid_3->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), ((void**)&pID3D12Device__uid_3_resetAllocator));
    NV_CHECK_RESULT(result);

    result = pID3D12Device__uid_3->CreateCommandList(0u, D3D12_COMMAND_LIST_TYPE_DIRECT, pID3D12Device__uid_3_resetAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), ((void**)&pID3D12Device__uid_3_resetCommandList));
    NV_CHECK_RESULT(result);

    // Use an offscreen surface to render to, rather than the real backbuffer.
    NV_SAFE_RELEASE(pID3D12Resource__uid_16);
    pID3D12Resource__uid_16 = pIDXGISwapChain_uid_11_offscreen_surface;
    pID3D12Resource__uid_16->AddRef();

    // Use an offscreen surface to render to, rather than the real backbuffer.
    NV_SAFE_RELEASE(pID3D12Resource__uid_17);
    pID3D12Resource__uid_17 = pIDXGISwapChain_uid_11_offscreen_surface;
    pID3D12Resource__uid_17->AddRef();

    //-------------------------------------------------------------------------
    // Initializing descriptor heap: pID3D12DescriptorHeap__uid_14
    //-------------------------------------------------------------------------
    pID3D12Device__uid_3->CreateRenderTargetView(pID3D12Resource__uid_16, D3D12InitRTVTexture2D(pID3D12Device__uid_3_rtv, DXGI_FORMAT_R8G8B8A8_UNORM, 0u, 0u), OffsetCPUDescriptor(0u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_14));
    pID3D12Device__uid_3->CreateRenderTargetView(pID3D12Resource__uid_17, D3D12InitRTVTexture2D(pID3D12Device__uid_3_rtv, DXGI_FORMAT_R8G8B8A8_UNORM, 0u, 0u), OffsetCPUDescriptor(1u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_14));
    pID3D12Device__uid_3->CreateRenderTargetView(pID3D12Resource__uid_25, D3D12InitRTVTexture2D(pID3D12Device__uid_3_rtv, DXGI_FORMAT_R8G8B8A8_UNORM, 0u, 0u), OffsetCPUDescriptor(2u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_14));

    //-------------------------------------------------------------------------
    // Initializing descriptor heap: pID3D12DescriptorHeap__uid_15
    //-------------------------------------------------------------------------
    pID3D12Device__uid_3->CreateDepthStencilView(pID3D12Resource__uid_18, D3D12InitDSVTexture2D(pID3D12Device__uid_3_dsv, DXGI_FORMAT_D24_UNORM_S8_UINT, 0u), OffsetCPUDescriptor(0u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_15));

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_85[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_18, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_85);
    }

    // Initialize pID3D12Resource__uid_18
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_21[2] = { {50}, {51} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_86[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_18, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_18, pID3D12Resource__uid_18_uploadDataBuffer, 2u, Serialization_DATABASE_HANDLE_temp_21, 1ull, D3D12_RESOURCE_BARRIER_temp_86);

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_22[2] = { {50}, {51} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_87[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_18, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_resetCommandList, pID3D12Resource__uid_18, pID3D12Resource__uid_18_uploadDataBuffer, 2u, Serialization_DATABASE_HANDLE_temp_22, 1ull, D3D12_RESOURCE_BARRIER_temp_87);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_88[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_19, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_88);
    }

    // Initialize pID3D12Resource__uid_19
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_23[1] = { {52} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_89[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_19, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_19, pID3D12Resource__uid_19_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_23, 1ull, D3D12_RESOURCE_BARRIER_temp_89);

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_24[1] = { {52} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_90[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_19, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_resetCommandList, pID3D12Resource__uid_19, pID3D12Resource__uid_19_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_24, 1ull, D3D12_RESOURCE_BARRIER_temp_90);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_91[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_20, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_91);
    }

    // Initialize pID3D12Resource__uid_20
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_25[1] = { {53} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_92[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_20, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_20, pID3D12Resource__uid_20_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_25, 1ull, D3D12_RESOURCE_BARRIER_temp_92);

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_26[1] = { {53} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_93[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_20, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_resetCommandList, pID3D12Resource__uid_20, pID3D12Resource__uid_20_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_26, 1ull, D3D12_RESOURCE_BARRIER_temp_93);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_94[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_21, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_94);
    }

    // Initialize pID3D12Resource__uid_21
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_27[1] = { {54} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_95[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_21, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_21, pID3D12Resource__uid_21_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_27, 1ull, D3D12_RESOURCE_BARRIER_temp_95);
    }

    // Initialize pID3D12Resource__uid_24
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_28[1] = { {55} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_96[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_24, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_24, pID3D12Resource__uid_24_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_28, 1ull, D3D12_RESOURCE_BARRIER_temp_96);

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_29[1] = { {55} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_97[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_24, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_resetCommandList, pID3D12Resource__uid_24, pID3D12Resource__uid_24_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_29, 1ull, D3D12_RESOURCE_BARRIER_temp_97);
    }

    // Initialize pID3D12Resource__uid_25
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_30[1] = { {56} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_98[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_25, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_25, pID3D12Resource__uid_25_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_30, 1ull, D3D12_RESOURCE_BARRIER_temp_98);

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_31[1] = { {56} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_99[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_25, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_resetCommandList, pID3D12Resource__uid_25, pID3D12Resource__uid_25_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_31, 1ull, D3D12_RESOURCE_BARRIER_temp_99);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_100[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_26, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_100);
    }

    // Initialize pID3D12Resource__uid_26
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_32[10] = { 
            {57}, {58}, {59}, {60}, {61}, {62}, {63}, {64}, 
            {65}, {66} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_101[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_26, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_26, pID3D12Resource__uid_26_uploadDataBuffer, 10u, Serialization_DATABASE_HANDLE_temp_32, 1ull, D3D12_RESOURCE_BARRIER_temp_101);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_102[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_28, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_102);
    }

    // Initialize pID3D12Resource__uid_28
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_33[9] = { 
            {67}, {68}, {69}, {70}, {71}, {72}, {73}, {74}, 
            {75} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_103[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_28, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_28, pID3D12Resource__uid_28_uploadDataBuffer, 9u, Serialization_DATABASE_HANDLE_temp_33, 1ull, D3D12_RESOURCE_BARRIER_temp_103);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_104[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_30, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_104);
    }

    // Initialize pID3D12Resource__uid_30
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_34[10] = { 
            {76}, {77}, {78}, {79}, {80}, {81}, {82}, {83}, 
            {84}, {85} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_105[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_30, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_30, pID3D12Resource__uid_30_uploadDataBuffer, 10u, Serialization_DATABASE_HANDLE_temp_34, 1ull, D3D12_RESOURCE_BARRIER_temp_105);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_106[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_32, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_106);
    }

    // Initialize pID3D12Resource__uid_32
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_35[1] = { {86} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_107[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_32, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_32, pID3D12Resource__uid_32_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_35, 1ull, D3D12_RESOURCE_BARRIER_temp_107);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_108[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_34, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_108);
    }

    // Initialize pID3D12Resource__uid_34
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_36[1] = { {87} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_109[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_34, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_34, pID3D12Resource__uid_34_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_36, 1ull, D3D12_RESOURCE_BARRIER_temp_109);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_110[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_36, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_110);
    }

    // Initialize pID3D12Resource__uid_36
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_37[1] = { {88} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_111[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_36, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_36, pID3D12Resource__uid_36_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_37, 1ull, D3D12_RESOURCE_BARRIER_temp_111);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_112[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_38, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_112);
    }

    // Initialize pID3D12Resource__uid_38
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_38[1] = { {89} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_113[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_38, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_38, pID3D12Resource__uid_38_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_38, 1ull, D3D12_RESOURCE_BARRIER_temp_113);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_114[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_40, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_114);
    }

    // Initialize pID3D12Resource__uid_40
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_39[1] = { {90} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_115[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_40, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_40, pID3D12Resource__uid_40_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_39, 1ull, D3D12_RESOURCE_BARRIER_temp_115);
    }

    // Transition to initial state
    {
        BEGIN_DATA_SCOPE();

        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_116[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_42, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ, 4294967295u) };
        pID3D12Device__uid_3_initCommandList->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_116);
    }

    // Initialize pID3D12Resource__uid_42
    {
        BEGIN_DATA_SCOPE();

        static Serialization::DATABASE_HANDLE Serialization_DATABASE_HANDLE_temp_40[1] = { {91} };
        static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_117[1] = { D3D12TransitionBarrier(pID3D12Resource__uid_42, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST, 4294967295u) };
        D3D12InitResourceData(pID3D12Device__uid_3, pID3D12Device__uid_3_queue, pID3D12Device__uid_3_initCommandList, pID3D12Resource__uid_42, pID3D12Resource__uid_42_uploadDataBuffer, 1u, Serialization_DATABASE_HANDLE_temp_40, 1ull, D3D12_RESOURCE_BARRIER_temp_117);
    }

    //-------------------------------------------------------------------------
    // Apply initial data for pID3D12Resource__uid_45
    //-------------------------------------------------------------------------

    result = pID3D12Resource__uid_45->Map(0u, NULL, &pID3D12Resource__uid_45_mapped_subresource_0);
    NV_CHECK_RESULT(result);

    if (pID3D12Resource__uid_45_mapped_subresource_0)
    {
        BEGIN_DATA_SCOPE();

        memcpy(pID3D12Resource__uid_45_mapped_subresource_0, NV_GET_RESOURCE(void*, 92), 1280ull);
    }
    //-------------------------------------------------------------------------
    // Apply initial data for pID3D12Resource__uid_46
    //-------------------------------------------------------------------------

    result = pID3D12Resource__uid_46->Map(0u, NULL, &pID3D12Resource__uid_46_mapped_subresource_0);
    NV_CHECK_RESULT(result);

    if (pID3D12Resource__uid_46_mapped_subresource_0)
    {
        BEGIN_DATA_SCOPE();

        memcpy(pID3D12Resource__uid_46_mapped_subresource_0, NV_GET_RESOURCE(void*, 93), 768ull);
    }
    //-------------------------------------------------------------------------
    // Apply initial data for pID3D12Resource__uid_47
    //-------------------------------------------------------------------------

    result = pID3D12Resource__uid_47->Map(0u, NULL, &pID3D12Resource__uid_47_mapped_subresource_0);
    NV_CHECK_RESULT(result);

    if (pID3D12Resource__uid_47_mapped_subresource_0)
    {
        BEGIN_DATA_SCOPE();

        memcpy(pID3D12Resource__uid_47_mapped_subresource_0, NV_GET_RESOURCE(void*, 94), 768ull);
    }
    //-------------------------------------------------------------------------
    // Initializing descriptor heap: pID3D12DescriptorHeap__uid_56
    //-------------------------------------------------------------------------
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_26, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_BC3_UNORM, 5768u, 0u, 10u, 0u, 0.0f), OffsetCPUDescriptor(0u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_28, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_BC1_UNORM, 5768u, 0u, 9u, 0u, 0.0f), OffsetCPUDescriptor(1u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_30, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_BC3_UNORM, 5768u, 0u, 10u, 0u, 0.0f), OffsetCPUDescriptor(2u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_19, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_R32_FLOAT, 5768u, 0u, 1u, 0u, 0.0f), OffsetCPUDescriptor(3u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_20, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_R32_FLOAT, 5768u, 0u, 1u, 0u, 0.0f), OffsetCPUDescriptor(4u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_21, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_R32_FLOAT, 5768u, 0u, 1u, 0u, 0.0f), OffsetCPUDescriptor(5u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateUnorderedAccessView(pID3D12Resource__uid_19, NULL, D3D12InitUAVTexture2D(pID3D12Device__uid_3_uav, DXGI_FORMAT_R32_FLOAT, 0u, 0u), OffsetCPUDescriptor(6u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateUnorderedAccessView(pID3D12Resource__uid_20, NULL, D3D12InitUAVTexture2D(pID3D12Device__uid_3_uav, DXGI_FORMAT_R32_FLOAT, 0u, 0u), OffsetCPUDescriptor(7u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateUnorderedAccessView(pID3D12Resource__uid_21, NULL, D3D12InitUAVTexture2D(pID3D12Device__uid_3_uav, DXGI_FORMAT_R32_FLOAT, 0u, 0u), OffsetCPUDescriptor(8u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_24, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_R8G8B8A8_UNORM, 5768u, 0u, 1u, 0u, 0.0f), OffsetCPUDescriptor(9u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateUnorderedAccessView(pID3D12Resource__uid_24, NULL, D3D12InitUAVTexture2D(pID3D12Device__uid_3_uav, DXGI_FORMAT_R8G8B8A8_UNORM, 0u, 0u), OffsetCPUDescriptor(10u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    pID3D12Device__uid_3->CreateShaderResourceView(pID3D12Resource__uid_25, D3D12InitSRVTexture2D(pID3D12Device__uid_3_srv, DXGI_FORMAT_R8G8B8A8_UNORM, 5768u, 0u, 1u, 0u, 0.0f), OffsetCPUDescriptor(11u, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));

    result = pID3D12Device__uid_3_initCommandList->Close();
    NV_CHECK_RESULT(result);

    result = pID3D12Device__uid_3_resetCommandList->Close();
    NV_CHECK_RESULT(result);

    pID3D12Device__uid_3_queue->ExecuteCommandLists(1, ((ID3D12CommandList**)&pID3D12Device__uid_3_initCommandList));
    D3D12Finish(pID3D12Device__uid_3_queue, pID3D12Device__uid_3_fence, pID3D12Device__uid_3_fenceValue);
    NV_SAFE_RELEASE(pID3D12Device__uid_3_initCommandList);
    NV_SAFE_RELEASE(pID3D12Device__uid_3_initAllocator);
    NV_SAFE_RELEASE(pID3D12Resource__uid_21_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_26_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_28_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_30_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_32_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_34_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_36_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_38_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_40_uploadDataBuffer);
    NV_SAFE_RELEASE(pID3D12Resource__uid_42_uploadDataBuffer);
    FreeCachedMemory();
}
