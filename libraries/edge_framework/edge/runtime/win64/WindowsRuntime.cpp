#include "WindowsRuntime.h"

#include "../common/GlfwWindow.h"
#include "../HeadlessWIndow.h"

#include "WindowsRuntimeContext.h"

namespace edge
{
	namespace win32
	{
		WindowsRuntime::WindowsRuntime(const IRuntimeContext& context) :
			Runtime(context)
		{

		}

		void WindowsRuntime::CreateWindow(const Window::Properties& properties)
		{
			if (properties.mode == Window::Mode::Headless)
			{
				window = std::make_unique<HeadlessWindow>(properties);
			}
			else
			{
				window = std::make_unique<GlfwWindow>(this, properties);
			}
		}
	}
}