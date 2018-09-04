//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: CommandList00.cpp
// 
//-----------------------------------------------------------------------------

#include "Resources.h"
#include "Helpers.h"

//-----------------------------------------------------------------------------
// SetupCommandlist10Instance0
//-----------------------------------------------------------------------------
ID3D12GraphicsCommandList* SetupCommandlist10Instance0()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    static bool ready = false;
    if (ready)
    {
        BEGIN_DATA_SCOPE();

        return ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0);
    }

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetDescriptorHeaps(1u, &pID3D12DescriptorHeap__uid_56);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_87);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootSignature(pID3D12RootSignature__uid_58);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRoot32BitConstants(0u, 1u, NV_GET_RESOURCE(UINT*, 47), 3u);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRoot32BitConstants(0u, 2u, NV_GET_RESOURCE(UINT*, 48), 4u);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootDescriptorTable(3u, OffsetGPUDescriptor(6ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_9 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_9 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_9 = {pID3D12Resource__uid_19, 4294967295u, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_62;
        D3D12_RESOURCE_BARRIER_temp_62.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_9;
        D3D12_RESOURCE_BARRIER_temp_62.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_9;
        D3D12_RESOURCE_BARRIER_temp_62.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_9;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_63[1] = { D3D12_RESOURCE_BARRIER_temp_62 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_63);

    // Dispatch #0 [0...2]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->Dispatch(1u, 1u, 1u);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_89);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootSignature(pID3D12RootSignature__uid_58);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRoot32BitConstants(0u, 3u, NV_GET_RESOURCE(UINT*, 49), 0u);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootDescriptorTable(1u, OffsetGPUDescriptor(8ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootDescriptorTable(2u, OffsetGPUDescriptor(6ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootDescriptorTable(3u, OffsetGPUDescriptor(7ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));

    // Dispatch #1 [0...2]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->Dispatch(16u, 16u, 1u);

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_10 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_10 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_10 = {pID3D12Resource__uid_20, 4294967295u, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_65;
        D3D12_RESOURCE_BARRIER_temp_65.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_10;
        D3D12_RESOURCE_BARRIER_temp_65.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_10;
        D3D12_RESOURCE_BARRIER_temp_65.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_10;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_66[1] = { D3D12_RESOURCE_BARRIER_temp_65 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_66);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_69);

    static D3D12_VIEWPORT D3D12_VIEWPORT_temp_2[1] = { {0.0f, 0.0f, HexToFloat(0x44A00000/*1280.0f*/), HexToFloat(0x44340000/*720.0f*/), 0.0f, HexToFloat(0x3F800000/*1.0f*/)} };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->RSSetViewports(1u, D3D12_VIEWPORT_temp_2);

    static D3D12_RECT D3D12_RECT_temp_2[1] = { {0, 0, 1280, 720} };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->RSSetScissorRects(1u, D3D12_RECT_temp_2);

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_11 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_11 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_11 = {pID3D12Resource__uid_25, 4294967295u, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_68;
        D3D12_RESOURCE_BARRIER_temp_68.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_11;
        D3D12_RESOURCE_BARRIER_temp_68.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_11;
        D3D12_RESOURCE_BARRIER_temp_68.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_11;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_69[1] = { D3D12_RESOURCE_BARRIER_temp_68 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_69);

    // Clear #0 [0...1]
    static FLOAT FLOAT_temp_2[4] = { HexToFloat(0x3F333333/*0.7f*/), HexToFloat(0x3F333333/*0.7f*/), HexToFloat(0x3F333333/*0.7f*/), HexToFloat(0x3F800000/*1.0f*/) };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ClearRenderTargetView(OffsetCPUDescriptor(2ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_14), FLOAT_temp_2, 0u, NULL);

    // Clear #1 [0...1]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ClearDepthStencilView(OffsetCPUDescriptor(0ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_15), D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, HexToFloat(0x3F800000/*1.0f*/), 0, 0u, NULL);

    static D3D12_CPU_DESCRIPTOR_HANDLE D3D12_CPU_DESCRIPTOR_HANDLE_temp_5[] = {OffsetCPUDescriptor(2ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_14)};
    static D3D12_CPU_DESCRIPTOR_HANDLE D3D12_CPU_DESCRIPTOR_HANDLE_temp_6 = OffsetCPUDescriptor(0ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_15);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->OMSetRenderTargets(1u, D3D12_CPU_DESCRIPTOR_HANDLE_temp_5, TRUE, &D3D12_CPU_DESCRIPTOR_HANDLE_temp_6);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootSignature(pID3D12RootSignature__uid_57);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootConstantBufferView(3u, (pID3D12Resource__uid_45->GetGPUVirtualAddress()+0ull));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootDescriptorTable(4u, OffsetGPUDescriptor(4ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));

    static D3D12_VERTEX_BUFFER_VIEW D3D12_VERTEX_BUFFER_VIEW_temp_4[] = { {(pID3D12Resource__uid_36->GetGPUVirtualAddress()+0ull), 80000u, 32u} };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetVertexBuffers(0u, 1u, D3D12_VERTEX_BUFFER_VIEW_temp_4);

    static D3D12_INDEX_BUFFER_VIEW D3D12_INDEX_BUFFER_VIEW_temp_4 = {(pID3D12Resource__uid_38->GetGPUVirtualAddress()+0ull), 28812u, DXGI_FORMAT_R16_UINT};
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetIndexBuffer(&D3D12_INDEX_BUFFER_VIEW_temp_4);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootDescriptorTable(0u, OffsetGPUDescriptor(0ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootConstantBufferView(1u, (pID3D12Resource__uid_46->GetGPUVirtualAddress()+256ull));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootConstantBufferView(2u, (pID3D12Resource__uid_47->GetGPUVirtualAddress()+0ull));

    // Draw #0 [0...3]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->DrawIndexedInstanced(14406u, 1u, 0u, 0, 0u);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_78);

    static D3D12_VERTEX_BUFFER_VIEW D3D12_VERTEX_BUFFER_VIEW_temp_5[] = { {(pID3D12Resource__uid_32->GetGPUVirtualAddress()+0ull), 36864u, 32u} };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetVertexBuffers(0u, 1u, D3D12_VERTEX_BUFFER_VIEW_temp_5);

    static D3D12_INDEX_BUFFER_VIEW D3D12_INDEX_BUFFER_VIEW_temp_5 = {(pID3D12Resource__uid_34->GetGPUVirtualAddress()+0ull), 4608u, DXGI_FORMAT_R16_UINT};
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetIndexBuffer(&D3D12_INDEX_BUFFER_VIEW_temp_5);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootDescriptorTable(0u, OffsetGPUDescriptor(2ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootConstantBufferView(1u, (pID3D12Resource__uid_46->GetGPUVirtualAddress()+512ull));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootConstantBufferView(2u, (pID3D12Resource__uid_47->GetGPUVirtualAddress()+512ull));

    // Draw #1 [0...3]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->DrawIndexedInstanced(2304u, 1u, 0u, 0, 0u);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_75);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_81);

    static D3D12_VERTEX_BUFFER_VIEW D3D12_VERTEX_BUFFER_VIEW_temp_6[] = { {(pID3D12Resource__uid_40->GetGPUVirtualAddress()+0ull), 2097152u, 32u} };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetVertexBuffers(0u, 1u, D3D12_VERTEX_BUFFER_VIEW_temp_6);

    static D3D12_INDEX_BUFFER_VIEW D3D12_INDEX_BUFFER_VIEW_temp_6 = {(pID3D12Resource__uid_42->GetGPUVirtualAddress()+0ull), 1560600u, DXGI_FORMAT_R32_UINT};
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetIndexBuffer(&D3D12_INDEX_BUFFER_VIEW_temp_6);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootDescriptorTable(0u, OffsetGPUDescriptor(1ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootConstantBufferView(1u, (pID3D12Resource__uid_46->GetGPUVirtualAddress()+0ull));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootConstantBufferView(2u, (pID3D12Resource__uid_47->GetGPUVirtualAddress()+256ull));

    // Draw #2 [0...3]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->DrawIndexedInstanced(390150u, 1u, 0u, 0, 0u);

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_12 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_12 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_12 = {pID3D12Resource__uid_25, 4294967295u, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_71;
        D3D12_RESOURCE_BARRIER_temp_71.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_12;
        D3D12_RESOURCE_BARRIER_temp_71.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_12;
        D3D12_RESOURCE_BARRIER_temp_71.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_12;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_72[1] = { D3D12_RESOURCE_BARRIER_temp_71 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_72);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootSignature(pID3D12RootSignature__uid_59);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_91);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootDescriptorTable(0u, OffsetGPUDescriptor(11ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetComputeRootDescriptorTable(2u, OffsetGPUDescriptor(10ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_13 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_13 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_13 = {pID3D12Resource__uid_24, 4294967295u, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_74;
        D3D12_RESOURCE_BARRIER_temp_74.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_13;
        D3D12_RESOURCE_BARRIER_temp_74.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_13;
        D3D12_RESOURCE_BARRIER_temp_74.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_13;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_75[1] = { D3D12_RESOURCE_BARRIER_temp_74 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_75);

    // Dispatch #2 [0...2]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->Dispatch(80u, 45u, 1u);

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_14 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_14 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_14 = {pID3D12Resource__uid_24, 4294967295u, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_77;
        D3D12_RESOURCE_BARRIER_temp_77.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_14;
        D3D12_RESOURCE_BARRIER_temp_77.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_14;
        D3D12_RESOURCE_BARRIER_temp_77.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_14;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_78[1] = { D3D12_RESOURCE_BARRIER_temp_77 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_78);

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_15 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_15 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_15 = {pID3D12Resource__uid_16, 4294967295u, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_80;
        D3D12_RESOURCE_BARRIER_temp_80.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_15;
        D3D12_RESOURCE_BARRIER_temp_80.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_15;
        D3D12_RESOURCE_BARRIER_temp_80.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_15;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_81[1] = { D3D12_RESOURCE_BARRIER_temp_80 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_81);

    static D3D12_CPU_DESCRIPTOR_HANDLE D3D12_CPU_DESCRIPTOR_HANDLE_temp_7[] = {OffsetCPUDescriptor(0ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_14)};
    static D3D12_CPU_DESCRIPTOR_HANDLE D3D12_CPU_DESCRIPTOR_HANDLE_temp_8 = OffsetCPUDescriptor(0ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_15);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->OMSetRenderTargets(1u, D3D12_CPU_DESCRIPTOR_HANDLE_temp_7, TRUE, &D3D12_CPU_DESCRIPTOR_HANDLE_temp_8);

    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootSignature(pID3D12RootSignature__uid_59);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetPipelineState(pID3D12PipelineState__uid_84);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootDescriptorTable(0u, OffsetGPUDescriptor(11ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->SetGraphicsRootDescriptorTable(1u, OffsetGPUDescriptor(9ull, pID3D12Device__uid_3, pID3D12DescriptorHeap__uid_56));
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetVertexBuffers(0u, 1u, NULL);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetIndexBuffer(NULL);
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw #3 [0...3]
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->DrawInstanced(6u, 1u, 0u, 0u);

    static D3D12_RESOURCE_BARRIER_TYPE D3D12_RESOURCE_BARRIER_TYPE_temp_16 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    static D3D12_RESOURCE_BARRIER_FLAGS D3D12_RESOURCE_BARRIER_FLAGS_temp_16 = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    static D3D12_RESOURCE_TRANSITION_BARRIER D3D12_RESOURCE_TRANSITION_BARRIER_temp_16 = {pID3D12Resource__uid_16, 4294967295u, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON};
    D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_83;
        D3D12_RESOURCE_BARRIER_temp_83.Type = D3D12_RESOURCE_BARRIER_TYPE_temp_16;
        D3D12_RESOURCE_BARRIER_temp_83.Flags = D3D12_RESOURCE_BARRIER_FLAGS_temp_16;
        D3D12_RESOURCE_BARRIER_temp_83.Transition = D3D12_RESOURCE_TRANSITION_BARRIER_temp_16;

    static D3D12_RESOURCE_BARRIER D3D12_RESOURCE_BARRIER_temp_84[1] = { D3D12_RESOURCE_BARRIER_temp_83 };
    ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->ResourceBarrier(1u, D3D12_RESOURCE_BARRIER_temp_84);

    result = ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0)->Close();
    NV_CHECK_RESULT(result);

    ready = true;
    return ((ID3D12GraphicsCommandList*)pID3D12CommandList__uid_10_instance_0);
}
