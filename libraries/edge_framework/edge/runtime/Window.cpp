#include "Window.h"

namespace edge
{
	Window::Window(const Properties& properties) :
		properties{ properties }
	{
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

	bool Window::GetDisplayPresentInfo(VkDisplayPresentInfoKHR* info,
		uint32_t src_width, uint32_t src_height) const
	{
		// Default is to not use the extra present info
		return false;
	}
}