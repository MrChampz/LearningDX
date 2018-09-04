//--------------------------------------------------------------------------------------
// File: Threading.cpp
//
// Distributed as part of NVIDIA Nsight serialization output.
//
// Copyright (c) NVIDIA Corporation.  All rights reserved.
//--------------------------------------------------------------------------------------

#include "Threading.h"
#include "Helpers.h"

//--------------------------------------------------------------------------------------
// ThreadStartParams - Struct used to pass the params to the thread startup function
//--------------------------------------------------------------------------------------
struct ThreadStartParams
{
    ThreadRunner* threadRunner;
    HANDLE sequenceEvent;
    HANDLE doneEvent;
    bool* exitReplayerThreads;
};

#define WAIT_FOR_SIGNAL() \
    WaitForSingleObject(params->sequenceEvent, INFINITE)

#define SIGNAL_THREAD_DONE() \
    SetEvent(params->doneEvent)

#define RELEASE_THREAD_RESOURCES()      \
    CloseHandle(params->sequenceEvent); \
    CloseHandle(params->doneEvent)

//--------------------------------------------------------------------------------------
// ThreadStart - The method each replayer thread begins running
//--------------------------------------------------------------------------------------
DWORD WINAPI ThreadStart(LPVOID lpParameter)
{
    ThreadStartParams* params = (ThreadStartParams*)lpParameter;

    bool firstTime = true;

    while (true)
    {
        WAIT_FOR_SIGNAL();

        if (firstTime)
        {
            params->threadRunner->Init();
            firstTime = false;
        }

        if (*params->exitReplayerThreads)
        {
            SIGNAL_THREAD_DONE();
            break;
        }

        params->threadRunner->RunThread();
        SIGNAL_THREAD_DONE();
    }

    RELEASE_THREAD_RESOURCES();

    delete params;

    return EXIT_SUCCESS;
}

//--------------------------------------------------------------------------------------
// CreateThreadEvents - Create Windows events used to synchronize the threads
//--------------------------------------------------------------------------------------
bool CreateThreadEvents(unsigned int numThreads, HANDLE* events)
{
    for (unsigned int currentEvent = 0; currentEvent < numThreads; ++currentEvent)
    {
        HANDLE eventHandle = CreateEvent(NULL, // Cannot be inherited
            FALSE, // Auto reset object
            FALSE, // Initially not signaled
            NULL); // No need for a name

        if (!eventHandle)
        {
            for (unsigned int existingHandle = 0; existingHandle < currentEvent; ++existingHandle)
            {
                CloseHandle(events[existingHandle]);
            }

            return false;
        }

        events[currentEvent] = eventHandle;
    }

    return true;
}

//--------------------------------------------------------------------------------------
// CreateReplayerThreads - Create and start the replayer threads
//--------------------------------------------------------------------------------------
bool CreateReplayerThreads(ThreadRunner** threadRunners, unsigned int numThreads, HANDLE* threadSequenceEvents, HANDLE* threadDoneEvents, bool& exitReplayerThreads)
{
    for (unsigned int currentThread = 0; currentThread < numThreads; ++currentThread)
    {
        ThreadStartParams* params = new ThreadStartParams();
        {
            params->threadRunner = threadRunners[currentThread];
            params->sequenceEvent = threadSequenceEvents[currentThread];
            params->doneEvent = threadDoneEvents[currentThread];
            params->exitReplayerThreads = &exitReplayerThreads;
        }

        // Create the thread
        HANDLE threadHandle = CreateThread(NULL, // Cannot be inherited
            0, // Default stack size
            ThreadStart, // The function to run
            (LPVOID)params, // The arguments
            0, // Start immediately
            NULL); // No need for the Windows thread id

        if (!threadHandle)
        {
            for (unsigned int existingHandle = currentThread; existingHandle < numThreads; ++existingHandle)
            {
                CloseHandle(threadSequenceEvents[existingHandle]);
                CloseHandle(threadDoneEvents[existingHandle]);
            }

            delete params;
            ExitReplayerThreads(currentThread, threadSequenceEvents, threadDoneEvents, exitReplayerThreads);
            return false;
        }

        CloseHandle(threadHandle);
    }

    return true;
}

//--------------------------------------------------------------------------------------
// ExitReplayerThreads - Exit all of the replayer threads
//--------------------------------------------------------------------------------------
void ExitReplayerThreads(unsigned int numThreads, HANDLE* threadSequenceEvents, HANDLE* threadDoneEvents, bool& exitReplayerThreads)
{
    // Set the flag telling the threads to exit
    exitReplayerThreads = true;

    // Wake up each thread
    for (unsigned int i = 0; i < numThreads; ++i)
    {
        SetEvent(threadSequenceEvents[i]);
    }

    // Wait for each thread to exit
    WaitForMultipleObjects(numThreads, threadDoneEvents, TRUE, INFINITE);
}

//--------------------------------------------------------------------------------------
// RunReplayerThreads - Tell each replayer thread to run a frame and wait for
// them to complete
//--------------------------------------------------------------------------------------
void RunReplayerThreads(unsigned int numThreads, unsigned int startThread, HANDLE* threadDoneEvents, HANDLE* threadSequenceEvents)
{
    // Tell the first thread to begin the frame
    SetEvent(threadSequenceEvents[startThread]);

    // Wait for the threads to report that they are ready to run the post frame method
    WaitForMultipleObjects(numThreads, threadDoneEvents, TRUE, INFINITE);

    // Wake up the threads so they can run their post frame methods
    for (unsigned int i = 0; i < numThreads; ++i)
    {
        SetEvent(threadSequenceEvents[i]);
    }

    // Wait for all of the post frame methods to finish
    WaitForMultipleObjects(numThreads, threadDoneEvents, TRUE, INFINITE);
}

//--------------------------------------------------------------------------------------
// ThreadRunner implementations - Implementation of ThreadRunner class and derived
// classes
//--------------------------------------------------------------------------------------
ThreadRunner::ThreadRunner(HANDLE sequenceEvent, HANDLE doneEvent, RunFrameFunc runFrameFunc)
    : m_sequenceEvent(sequenceEvent)
    , m_doneEvent(doneEvent)
    , m_runFrameFunc(runFrameFunc)
{
}

void ThreadRunner::RunThread()
{
    PreRunThread();
    m_runFrameFunc();
    PostRunThread();
}

void ThreadRunner::SignalMainThreadAndWait() const
{
    SetEvent(m_doneEvent);
    WaitForSingleObject(m_sequenceEvent, INFINITE);
}

BasicThreadRunner::BasicThreadRunner(HANDLE sequenceEvent, HANDLE doneEvent, RunFrameFunc runFrameFunc, ResetFrameFunc resetFrameFunc)
    : ThreadRunner(sequenceEvent, doneEvent, runFrameFunc)
    , m_resetFrameFunc(resetFrameFunc)
{
}

void BasicThreadRunner::PostRunThread()
{
    // Make sure to let the main thread know we are here and wait for its permission
    // to continue.
    SignalMainThreadAndWait();
    if (hasFrameReset)
    {
        m_resetFrameFunc(false);
    }
}