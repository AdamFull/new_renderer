#pragma once

#include "../Runtime.h"

struct android_app;

namespace edge
{
	namespace android
	{
		class AndroidRuntimeContext;

		class AndroidRuntime final : public Runtime
		{
		public:
			AndroidRuntime(const RuntimeContext& context);
			virtual ~AndroidRuntime() = default;

			int32_t Initialize() override;

			void Terminate(int32_t code) override;

			void ProcessInputEvents();

			void SetSurfaceReady();

			android_app* GetAndroidApp();
			android_app const* GetAndroidApp() const;

		protected:
			virtual void CreateWindow(const Window::Properties& properties) override;
		private:
			const AndroidRuntimeContext* ctx{ nullptr };

			bool surface_ready{ false };
		};
	}
}