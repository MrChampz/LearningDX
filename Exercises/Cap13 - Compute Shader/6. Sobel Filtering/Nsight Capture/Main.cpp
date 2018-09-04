//-----------------------------------------------------------------------------
// Generated with NVIDIA Nsight Graphics 1.1
// 
// File: Main.cpp
// 
//-----------------------------------------------------------------------------

// Defines the entry point that initializes and runs the serialized frame capture

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "Helpers.h"
#include "ReplayProcedures.h"
#include "Resources.h"
#include "Threading.h"

//-----------------------------------------------------------------------------
// Replay Data
//-----------------------------------------------------------------------------
static int repeatCount = -1;
static bool automated = false;

//-----------------------------------------------------------------------------
// ProcessMessages
//-----------------------------------------------------------------------------
static void ProcessMessages(bool& quit)
{
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            quit = true;
        }
        else
        {
            // Translate and dispatch the message
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

//-----------------------------------------------------------------------------
// ParseInt
//-----------------------------------------------------------------------------
static bool ParseInt(const char* str, int& val)
{
    std::istringstream reader(str);
    return (reader >> val) && reader.get() == EOF;
}

//-----------------------------------------------------------------------------
// ParseCommandLine
//-----------------------------------------------------------------------------
static bool ParseCommandLine()
{
    for (int i = 1; i < __argc; ++i)
    {
        if (0 == strcmp(__argv[i], "-repeat"))
        {
            ++i;
            if (i >= __argc)
            {
                return false;
            }

            if (!ParseInt(__argv[i], repeatCount))
            {
                return false;
            }
        }
        else if (0 == strcmp(__argv[i], "-noreset"))
        {
            hasFrameReset = false;
        }
        else if (0 == strcmp(__argv[i], "-forcedebug"))
        {
            forceDebug = true;
        }
        else if (0 == strcmp(__argv[i], "-automated"))
        {
            automated = true;
        }
        else
        {
            // Unknown command
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
// Usage
//-----------------------------------------------------------------------------
static void Usage()
{
    const wchar_t* usage =
        L"Options:\n"
        L"-repeat N    -- Number of frames to run\n"
        L"-noreset     -- Do not perform a state reset in between frames\n"
        L"-automated   -- Do not use facilities that require user interaction\n"
    ;

    MessageBox(NULL, usage, L"Invalid command line", MB_ICONEXCLAMATION);
}

//-----------------------------------------------------------------------------
// WinMain
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    bool quit = false;

    currentInstance = hInstance;

    // Parse the command line arguments
    if (!ParseCommandLine())
    {
        Usage();
        return EXIT_FAILURE;
    }

    int ret = EXIT_SUCCESS;

    try
    {
        CreateAndSetupResources();

        int repeatIteration = 0;
        while (repeatCount == -1 || repeatIteration < repeatCount)
        {
            ProcessMessages(quit);
            if (quit)
            {
                break;
            }

            RunFrame();

            repeatIteration = (std::max)(0, repeatIteration+1);
        }
    }
    catch (std::exception& e)
    {
        if (automated) {
            fprintf(stderr, "Nvda.Replayer Error: %s", e.what());
        } else {
            std::string errorMessage(e.what());
            std::wstring errorMessageW(errorMessage.begin(), errorMessage.end());
            MessageBox(NULL, errorMessageW.c_str(), L"Nvda.Replayer Error", MB_ICONEXCLAMATION);
        }
        ret = EXIT_FAILURE;
    }

    ReleaseResources();
    return ret;
}
