#include "GlfwNativeSurface.h"

#if defined(PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include "../win64/WindowsNativeSurface.h"
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace edge
{
	NativeSurface* MakeGlfwNativeSurface(GLFWwindow* handle)
	{
		NativeSurface* surface{ nullptr };

#if defined(PLATFORM_WINDOWS)
		HWND hwnd = glfwGetWin32Window(handle);
		HINSTANCE hinst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
		surface = new win32::WindowsNativeSurface(hinst, hwnd);
#endif

		return surface;
	}
}