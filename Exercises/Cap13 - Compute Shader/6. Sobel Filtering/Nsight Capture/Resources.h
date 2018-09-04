//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: Resources.h
// 
//-----------------------------------------------------------------------------

#pragma once

#include <d3d9.h>
#include <d3d12.h>
#include <atlbase.h>
#include <vector>
#include <dxgi1_4.h>
#include <cstdint>

#include "CommandLists.h"

//-----------------------------------------------------------------------------
// Captured Resources
//-----------------------------------------------------------------------------
extern IDXGIFactory1* pIDXGIFactory1_uid_2;
extern ID3D12Device* pID3D12Device__uid_3;
extern ID3D12Fence* pID3D12Fence__uid_4;
extern ID3D12CommandQueue* pID3D12CommandQueue__uid_8;
extern ID3D12CommandList* pID3D12CommandList__uid_10_instance_0;
extern IDXGISwapChain* pIDXGISwapChain_uid_11;
extern ID3D12DescriptorHeap* pID3D12DescriptorHeap__uid_14;
extern ID3D12DescriptorHeap* pID3D12DescriptorHeap__uid_15;
extern ID3D12Resource* pID3D12Resource__uid_16;
extern ID3D12Resource* pID3D12Resource__uid_17;
extern ID3D12Resource* pID3D12Resource__uid_18;
extern ID3D12Resource* pID3D12Resource__uid_19;
extern ID3D12Resource* pID3D12Resource__uid_20;
extern ID3D12Resource* pID3D12Resource__uid_21;
extern ID3D12Resource* pID3D12Resource__uid_24;
extern ID3D12Resource* pID3D12Resource__uid_25;
extern ID3D12Resource* pID3D12Resource__uid_26;
extern ID3D12Resource* pID3D12Resource__uid_28;
extern ID3D12Resource* pID3D12Resource__uid_30;
extern ID3D12Resource* pID3D12Resource__uid_32;
extern ID3D12Resource* pID3D12Resource__uid_34;
extern ID3D12Resource* pID3D12Resource__uid_36;
extern ID3D12Resource* pID3D12Resource__uid_38;
extern ID3D12Resource* pID3D12Resource__uid_40;
extern ID3D12Resource* pID3D12Resource__uid_42;
extern ID3D12CommandAllocator* pID3D12CommandAllocator__uid_44;
extern ID3D12Resource* pID3D12Resource__uid_45;
extern ID3D12Resource* pID3D12Resource__uid_46;
extern ID3D12Resource* pID3D12Resource__uid_47;
extern ID3D12DescriptorHeap* pID3D12DescriptorHeap__uid_56;
extern ID3D12RootSignature* pID3D12RootSignature__uid_57;
extern ID3D12RootSignature* pID3D12RootSignature__uid_58;
extern ID3D12RootSignature* pID3D12RootSignature__uid_59;
extern ID3D12PipelineState* pID3D12PipelineState__uid_69;
extern ID3D12PipelineState* pID3D12PipelineState__uid_75;
extern ID3D12PipelineState* pID3D12PipelineState__uid_78;
extern ID3D12PipelineState* pID3D12PipelineState__uid_81;
extern ID3D12PipelineState* pID3D12PipelineState__uid_84;
extern ID3D12PipelineState* pID3D12PipelineState__uid_87;
extern ID3D12PipelineState* pID3D12PipelineState__uid_89;
extern ID3D12PipelineState* pID3D12PipelineState__uid_91;
extern HWND hwnd_0000000000180B4E;
extern ID3D12Resource* pIDXGISwapChain_uid_11_offscreen_surface;
extern std::vector<CComPtr<ID3D12GraphicsCommandList>> pIDXGISwapChain_uid_11_presentCommandLists;
extern std::vector<CComPtr<ID3D12CommandAllocator>> pIDXGISwapChain_uid_11_presentAllocators;
extern ID3D12Fence* pID3D12Device__uid_3_fence;
extern UINT64 pID3D12Device__uid_3_fenceValue;
extern ID3D12GraphicsCommandList* pID3D12Device__uid_3_initCommandList;
extern ID3D12CommandQueue* pID3D12Device__uid_3_queue;
extern ID3D12GraphicsCommandList* pID3D12Device__uid_3_resetCommandList;
extern ID3D12CommandAllocator* pID3D12Device__uid_3_initAllocator;
extern ID3D12CommandAllocator* pID3D12Device__uid_3_resetAllocator;
extern D3D12_RENDER_TARGET_VIEW_DESC pID3D12Device__uid_3_rtv;
extern D3D12_DEPTH_STENCIL_VIEW_DESC pID3D12Device__uid_3_dsv;
extern D3D12_SHADER_RESOURCE_VIEW_DESC pID3D12Device__uid_3_srv;
extern D3D12_UNORDERED_ACCESS_VIEW_DESC pID3D12Device__uid_3_uav;
extern ID3D12CommandAllocator* pID3D12CommandList__uid_10_instance_0_allocator;
extern ID3D12Resource* pID3D12Resource__uid_18_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_19_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_20_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_21_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_24_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_25_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_26_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_28_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_30_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_32_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_34_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_36_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_38_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_40_uploadDataBuffer;
extern ID3D12Resource* pID3D12Resource__uid_42_uploadDataBuffer;
extern void* pID3D12Resource__uid_45_mapped_subresource_0;
extern void* pID3D12Resource__uid_46_mapped_subresource_0;
extern void* pID3D12Resource__uid_47_mapped_subresource_0;
