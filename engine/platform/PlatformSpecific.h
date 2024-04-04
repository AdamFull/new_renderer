#pragma once

#include <standart_library.h>

#if defined(PLATFORM_WIN32)
#include <Windows.h>

#include "window/WindowWin32.h"
#include "window/WindowGLFW.h"
#endif

namespace engine
{
	inline stl::unique_ptr<IWindowInterface> make_platform_window(const FWindowCreateInfo& window_ci)
	{
		stl::unique_ptr<IWindowInterface> window_handle{};

#if defined(PLATFORM_WIN32)
		window_handle = stl::make_unique<platform::CWindowGLFW>();
#endif
		window_handle->create(window_ci);

		return window_handle;
	}
}