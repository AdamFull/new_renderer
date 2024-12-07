#pragma once

#include "../RuntimeContext.h"

#include <Windows.h>

#ifdef CreateWindow
#undef CreateWindow
#endif

namespace engine
{
	namespace win32
	{
		class WindowsRuntimeContext final : public RuntimeContext
		{
		public:
			WindowsRuntimeContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow);
			virtual ~WindowsRuntimeContext() = default;
		};
	}
}