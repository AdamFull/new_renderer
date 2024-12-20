#include "Window.h"

namespace edge
{
	Window::Window(const Properties& properties) :
		properties{ properties }
	{
	}

	NativeSurface* Window::GetNativeSurface()
	{
		return native_surface;
	}

	void Window::ProcessEvents()
	{
	}

	Window::Extent Window::Resize(const Extent& new_extent)
	{
		if (properties.resizable)
		{
			properties.extent.width = new_extent.width;
			properties.extent.height = new_extent.height;
		}

		return properties.extent;
	}

	const Window::Extent& Window::GetExtent() const
	{
		return properties.extent;
	}

	float Window::GetContentScaleFactor() const
	{
		return 1.0f;
	}

	Window::Mode Window::GetWindowMode() const
	{
		return properties.mode;
	}
}