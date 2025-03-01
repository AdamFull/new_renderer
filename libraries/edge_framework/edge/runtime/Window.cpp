#include "Window.h"

namespace edge
{
	Window::Window(const Properties& properties) :
		properties{ properties }
	{
	}

	auto Window::Resize(const Extent& new_extent) noexcept -> Window::Extent
	{
		if (properties.resizable)
		{
			properties.extent.width = new_extent.width;
			properties.extent.height = new_extent.height;
		}

		return properties.extent;
	}
}