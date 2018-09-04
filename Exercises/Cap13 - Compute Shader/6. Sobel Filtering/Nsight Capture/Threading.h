//--------------------------------------------------------------------------------------
// File: Threading.h
//
// Distributed as part of NVIDIA Nsight serialization output.
//
// Copyright (c) NVIDIA Corporation.  All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "Resources.h"
#include <Windows.h>

//--------------------------------------------------------------------------------------
// NV_EXEC - Wrapper macro for functions
// -------------------------------------------------------------------------------------
#define NV_EXEC(_Function) _Function

//--------------------------------------------------------------------------------------
// NV_SIGNAL_THREAD_AND_WAIT - Signal a waiting thread to begin execution and wait
// to be signaled
// -------------------------------------------------------------------------------------
#define NV_SIGNAL_THREAD_AND_WAIT(_ThreadId)   \
    SetEvent(threadSequenceEvents[_ThreadId]); \
    WaitForSingleObject(threadSequenceEvents[threadId], INFINITE)

//--------------------------------------------------------------------------------------
// NV_SIGNAL_FRAME_COMPLETE - Signal that the frame traversal has been completed
// -------------------------------------------------------------------------------------
#define NV_SIGNAL_FRAME_COMPLETE()                          \
    for (unsigned int i = 0; i < NUM_REPLAYER_THREADS; ++i) \
    {                                                       \
        if (i != threadId)                                  \
        {                                                   \
            SetEvent(threadSequenceEvents[i]);              \
        }                                                   \
    }

//--------------------------------------------------------------------------------------
// Function pointer types
//--------------------------------------------------------------------------------------
typedef void (*ThreadInitFunc)();
typedef void (*RunFrameFunc)();
typedef void (*ResetFrameFunc)(bool);

//--------------------------------------------------------------------------------------
// ThreadRunner - Class and derived classes used to manage the running of threads
//--------------------------------------------------------------------------------------
class ThreadRunner
{
public:
    ThreadRunner(HANDLE sequenceEvent, HANDLE doneEvent, RunFrameFunc runFrameFunc);
    virtual void Init()
    {
    }
    void RunThread();

protected:
    void SignalMainThreadAndWait() const;

private:
    virtual void PreRunThread()
    {
    }
    virtual void PostRunThread()
    {
    }
    HANDLE m_sequenceEvent;
    HANDLE m_doneEvent;
    RunFrameFunc m_runFrameFunc;
};

class BasicThreadRunner : public ThreadRunner
{
public:
    BasicThreadRunner(HANDLE sequenceEvent, HANDLE doneEvent, RunFrameFunc runFrameFunc, ResetFrameFunc resetFrameFunc);

private:
    virtual void PostRunThread();
    ResetFrameFunc m_resetFrameFunc;
};

//--------------------------------------------------------------------------------------
// ThreadStart - The method each replayer thread begins running
//--------------------------------------------------------------------------------------
DWORD WINAPI ThreadStart(LPVOID lpParameter);

//--------------------------------------------------------------------------------------
// CreateThreadEvents - Create Windows events used to synchronize the threads
//--------------------------------------------------------------------------------------
bool CreateThreadEvents(unsigned int numThreads, HANDLE* events);

//--------------------------------------------------------------------------------------
// CreateReplayerThreads - Create and start the replayer threads
//--------------------------------------------------------------------------------------
bool CreateReplayerThreads(ThreadRunner** threadRunners, unsigned int numThreads, HANDLE* threadSequenceEvents, HANDLE* threadDoneEvents, bool& exitReplayerThreads);

//--------------------------------------------------------------------------------------
// ExitReplayerThreads - Exit all of the replayer threads
//--------------------------------------------------------------------------------------
void ExitReplayerThreads(unsigned int numThreads, HANDLE* threadSequenceEvents, HANDLE* threadDoneEvents, bool& exitReplayerThreads);

//--------------------------------------------------------------------------------------
// RunReplayerThreads - Tell each replayer thread to run a frame and wait for
// them to complete
//--------------------------------------------------------------------------------------
void RunReplayerThreads(unsigned int numThreads, unsigned int startThread, HANDLE* threadDoneEvents, HANDLE* threadSequenceEvents);