//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: ReplayProcedures.cpp
// 
//-----------------------------------------------------------------------------

#include "Helpers.h"
#include "Resources.h"
#include "Threading.h"

//-----------------------------------------------------------------------------
// Replay Procedures
//-----------------------------------------------------------------------------
extern void ResetWinResources(bool firstTime);
extern void TerminateEventRanges();
extern void SetInitialFrameState00();
extern void SetupWinResources();
extern void RestoreEventRanges();
extern void ResetInitialFrameState00(bool firstTime);
extern void CreateResources00();
extern void CreateWinResources00();
extern void ReleaseResources00();
extern void ReleaseWinResources00();

extern void RunFrame0Part00();

//-----------------------------------------------------------------------------
// CreateAndSetupResources
//-----------------------------------------------------------------------------
void CreateAndSetupResources()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    CreateWinResources00();
    SetupWinResources();
    GetDatabase().SetForceEvict(true);
    CreateResources00();
    SetInitialFrameState00();
    GetDatabase().SetForceEvict(false);
    ResetInitialFrameState00(true);
}

//-----------------------------------------------------------------------------
// ReleaseResources
//-----------------------------------------------------------------------------
void ReleaseResources()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    ReleaseResources00();
    ReleaseWinResources00();
}

//-----------------------------------------------------------------------------
// RunFrame
//-----------------------------------------------------------------------------
void RunFrame()
{
    BEGIN_DATA_SCOPE_FUNCTION();

    RestoreEventRanges();
    RunFrame0Part00();
    TerminateEventRanges();
    if (hasFrameReset)
    {
        BEGIN_DATA_SCOPE();

        ResetWinResources(false);
        ResetInitialFrameState00(false);
    }
}
