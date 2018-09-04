//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: Frame0Part00.cpp
// 
//-----------------------------------------------------------------------------

#include "Resources.h"
#include "Helpers.h"
#include "Threading.h"

//-----------------------------------------------------------------------------
// RunFrame0Part00
//-----------------------------------------------------------------------------
void RunFrame0Part00()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    result = pID3D12CommandQueue__uid_8->Signal(pID3D12Fence__uid_4, 8831ull);
    NV_CHECK_RESULT(result);

    while (pID3D12Fence__uid_4->GetCompletedValue() < 8831)
    {
        BEGIN_DATA_SCOPE();

        ::Sleep(0);
    }

    if (pID3D12Resource__uid_45_mapped_subresource_0)
    {
        BEGIN_DATA_SCOPE();

        NV_EXEC(memcpy(pID3D12Resource__uid_45_mapped_subresource_0, NV_GET_RESOURCE(void*, 0), 1280ull));
    }
    if (pID3D12Resource__uid_47_mapped_subresource_0)
    {
        BEGIN_DATA_SCOPE();

        NV_EXEC(memcpy(pID3D12Resource__uid_47_mapped_subresource_0, NV_GET_RESOURCE(void*, 1), 768ull));
    }
    static ID3D12CommandList* pID3D12CommandList_temp_1[1] = { SetupCommandlist10Instance0() };
    pID3D12CommandQueue__uid_8->ExecuteCommandLists(1u, pID3D12CommandList_temp_1);

    D3D12OnPresent(((IDXGISwapChain3*)pIDXGISwapChain_uid_11), pIDXGISwapChain_uid_11_offscreen_surface, pID3D12Device__uid_3, pID3D12CommandQueue__uid_8, pIDXGISwapChain_uid_11_presentCommandLists, pIDXGISwapChain_uid_11_presentAllocators);

    D3D12Finish(pID3D12CommandQueue__uid_8, pID3D12Device__uid_3_fence, pID3D12Device__uid_3_fenceValue);

    result = pIDXGISwapChain_uid_11->Present(1u, 0u);
    NV_CHECK_PRESENT_RESULT(result, pIDXGISwapChain_uid_11);
}
