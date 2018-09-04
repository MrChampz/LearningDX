#include <Windows.h>
#include <D3DUtil.h>

#include "TessellationApp.h"

int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int showCmd)
{
	try
	{
		TessellationApp app(hInst);
		if (!app.Initialize())
		{
			return 0;
		}

		return app.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}