#include "Application.h"
#include <platform/PlatformSpecific.h>
#include "render/RenderInterface.h"

using namespace engine;

void get_platform_specific_window_data(FRenderInterfaceCreateInfo& rhi_ci, const std::unique_ptr<IWindowInterface>& window)
{
#if defined(PLATFORM_WIN32)
	auto platform_window = static_cast<platform::CWindowWin32*>(window.get());
	platform_window->getPlatformData(rhi_ci.hinstance, rhi_ci.hwnd);
#endif
}

void CApplication::create()
{
	FWindowCreateInfo window_ci{};
	window_ci.name = "Vulkan App";
	window_ci.flags = WINDOW_RESIZABLE_FLAG_BIT;
	window_ci.width = 800;
	window_ci.height = 600;

	window_handle = make_platform_window(window_ci);

	FRenderInterfaceCreateInfo rhi_ci{};
	rhi_ci.render_api = ERenderApi::eVulkan11;
	window_handle->getWindowSize(rhi_ci.initialWidth, rhi_ci.initialHeight);
	get_platform_specific_window_data(rhi_ci, window_handle);

	render_interface = std::make_unique<render::CRenderInterface>();
	render_interface->create(rhi_ci);
}

void CApplication::loop()
{
	while (window_handle->process())
	{
		if (window_handle->isResized())
		{
			// Wait while window is minimized
			while (window_handle->isMinimized())
			{
				if (!window_handle->process())
					return;
			}

			int32_t window_width{}, window_height{};
			window_handle->getWindowSize(window_width, window_height);
			render_interface->updateSurfaceSize(window_width, window_height);
		}

		render_interface->beginFrame();
		render_interface->endFrame();
	}
}