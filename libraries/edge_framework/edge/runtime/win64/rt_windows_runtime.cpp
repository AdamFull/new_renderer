#include "rt_windows_runtime.h"

#include "../common/rt_glfw_window.h"
#include "../rt_headless_wIndow.h"

#include "rt_windows_runtime_context.h"

namespace edge::win64
{
	WindowsRuntime::WindowsRuntime(const IRuntimeContext& context) :
		Runtime(context) {

	}

	void WindowsRuntime::create_window(const Window::Properties& properties) {
		if (properties.mode == Window::Mode::Headless) {
			window = std::make_unique<HeadlessWindow>(properties);
		}
		else {
			window = std::make_unique<GlfwWindow>(this, properties);
		}
	}
}