#pragma once

#include "../rt_window.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>

namespace edge::android
{
	class AndroidRuntime;

	class AndroidWindow final : public Window {
	public:
		AndroidWindow(AndroidRuntime& runtime, ANativeWindow*& window, const Window::Properties& properties);
		~AndroidWindow();

		bool should_close() const noexcept override;

		void process_events() override;

		void close() override;

		float get_dpi_factor() const noexcept override;

		void set_title(std::string_view new_title) const override;

		std::vector<const char*> get_required_surface_extensions() const override;
	private:
		AndroidRuntime& runtime;

		bool finish_called{ false };
	};
}