#pragma once

#include "../RuntimeContext.h"

#include <Windows.h>

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace edge
{
	namespace win32
	{
		class WindowsRuntimeContext final : public IRuntimeContext
		{
		public:
			WindowsRuntimeContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow);
			virtual ~WindowsRuntimeContext() = default;
		};
	}
}