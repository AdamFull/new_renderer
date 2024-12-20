#include "AndroidWindow.h"

#include "AndroidCommon.h"
#include "AndroidRuntime.h"

#include "AndroidNativeSurface.h"

namespace edge
{
	namespace android
	{
		AndroidWindow::AndroidWindow(AndroidRuntime& runtime, ANativeWindow*& window, const Window::Properties& properties) :
			Window{ properties },
			runtime{ runtime }
		{
			native_surface = new AndroidNativeSurface(window);
		}

		AndroidWindow::~AndroidWindow()
		{
			delete native_surface;
			native_surface = nullptr;
		}

		//vk::SurfaceKHR AndroidWindow::CreateSurface(vk::Instance instance, vk::AllocationCallbacks* callbacks)
		//{
		//	if (instance == VK_NULL_HANDLE || !handle || properties.mode == Mode::Headless)
		//	{
		//		return VK_NULL_HANDLE;
		//	}
		//
		//	vk::SurfaceKHR surface{};
		//
		//	vk::AndroidSurfaceCreateInfoKHR info{};
		//	info.window = handle;
		//
		//	auto result = instance.createAndroidSurfaceKHR(&info, callbacks, &surface);
		//
		//	return surface;
		//}

		bool AndroidWindow::ShouldClose()
		{
			return finish_called;
		}

		void AndroidWindow::ProcessEvents()
		{
			auto* app = runtime.GetApp();
			ProcessAndroidEvents(app);
		}

		void AndroidWindow::Close()
		{
			auto* app = runtime.GetApp();
			GameActivity_finish(app->activity);
			finish_called = true;
		}

		float AndroidWindow::GetDpiFactor() const
		{
			auto* app = runtime.GetApp();
			return AConfiguration_getDensity(app->config) / static_cast<float>(ACONFIGURATION_DENSITY_MEDIUM);
		}

		void AndroidWindow::SetTitle(std::string_view new_title)
		{

		}

		std::vector<const char*> AndroidWindow::GetRequiredSurfaceExtensions() const
		{
			return { VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };
		}
	}
}