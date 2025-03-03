#include "WindowsRuntimeContext.h"

#include <stdexcept>

namespace edge
{
	namespace win32
	{
		WindowsRuntimeContext::WindowsRuntimeContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) :
			IRuntimeContext{}
		{
			// Attempt to attach to the parent process console if it exists
			if (!AttachConsole(ATTACH_PARENT_PROCESS))
			{
				// No parent console, allocate a new one for this process
				if (!AllocConsole())
				{
					throw std::runtime_error{ "AllocConsole error" };
				}
			}

			FILE* fp;
			freopen_s(&fp, "conin$", "r", stdin);
			freopen_s(&fp, "conout$", "w", stdout);
			freopen_s(&fp, "conout$", "w", stderr);
		}
	}
}