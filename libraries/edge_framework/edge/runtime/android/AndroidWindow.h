#pragma once

#include "../Window.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>

namespace edge
{
	namespace android
	{
		class AndroidRuntime;

		class AndroidWindow final : public Window
		{
		public:
			AndroidWindow(AndroidRuntime& runtime, ANativeWindow*& window, const Window::Properties& properties);
			~AndroidWindow();

			bool ShouldClose() override;

			void ProcessEvents() override;

			void Close() override;

			float GetDpiFactor() const override;

			void SetTitle(std::string_view new_title) override;

			std::vector<const char*> GetRequiredSurfaceExtensions() const override;
		private:
			AndroidRuntime& runtime;

			bool finish_called{ false };
		};
	}
}