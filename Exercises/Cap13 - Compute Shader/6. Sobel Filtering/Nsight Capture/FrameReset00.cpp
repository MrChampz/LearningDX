//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: FrameReset00.cpp
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
// ResetInitialFrameState00
//-----------------------------------------------------------------------------
void ResetInitialFrameState00(bool firstTime)
{
    BEGIN_DATA_SCOPE_FUNCTION();

    D3DPERF_BeginEvent(0xFFC86464u, L"ResetInitialFrameState");
    pID3D12Device__uid_3_queue->ExecuteCommandLists(1, ((ID3D12CommandList**)&pID3D12Device__uid_3_resetCommandList));
    D3D12Finish(pID3D12Device__uid_3_queue, pID3D12Device__uid_3_fence, pID3D12Device__uid_3_fenceValue);

    // Reset to the frame start signal value.

    result = pID3D12Fence__uid_4->Signal(8830ull);
    NV_CHECK_RESULT(result);

    if (pID3D12Resource__uid_45_mapped_subresource_0)
    {
        BEGIN_DATA_SCOPE();

        memcpy(pID3D12Resource__uid_45_mapped_subresource_0, NV_GET_RESOURCE(void*, 95), 1280ull);
    }
    if (pID3D12Resource__uid_47_mapped_subresource_0)
    {
        BEGIN_DATA_SCOPE();

        memcpy(pID3D12Resource__uid_47_mapped_subresource_0, NV_GET_RESOURCE(void*, 96), 768ull);
    }
    D3DPERF_EndEvent();
}
