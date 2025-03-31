#include "rt_android_window.h"

#include "rt_android_common.h"
#include "rt_android_runtime.h"

#include "rt_android_native_surface.h"

namespace edge::android
{
	AndroidWindow::AndroidWindow(AndroidRuntime& runtime, ANativeWindow*& window, const Window::Properties& properties) :
		Window{ properties },
		runtime{ runtime } {
		native_surface = new AndroidNativeSurface(window);
	}

	AndroidWindow::~AndroidWindow() {
		delete native_surface;
		native_surface = nullptr;
	}

	bool AndroidWindow::should_close() const noexcept {
		return finish_called;
	}

	void AndroidWindow::process_events() {
		auto* app = runtime.get_android_app();
        process_android_events(app);
	}

	void AndroidWindow::close() {
        auto* app = runtime.get_android_app();
		GameActivity_finish(app->activity);
		finish_called = true;
	}

	float AndroidWindow::get_dpi_factor() const noexcept {
        auto* app = runtime.get_android_app();
		return AConfiguration_getDensity(app->config) / static_cast<float>(ACONFIGURATION_DENSITY_MEDIUM);
	}

	void AndroidWindow::set_title(std::string_view new_title) const {
	}

	std::vector<const char*> AndroidWindow::get_required_surface_extensions() const {
		return { "VK_KHR_android_surface" };
	}
}