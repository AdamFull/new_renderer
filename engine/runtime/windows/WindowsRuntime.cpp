#include <engine/runtime/windows/WindowsRuntime.h>

#include <engine/runtime/GlfwWindow.h>
#include <engine/runtime/HeadlessWIndow.h>

#include <engine/runtime/windows/WindowsRuntimeContext.h>

namespace engine
{
	namespace win32
	{
		WindowsRuntime::WindowsRuntime(const RuntimeContext& context) :
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