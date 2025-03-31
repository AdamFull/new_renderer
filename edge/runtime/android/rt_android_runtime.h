#pragma once

#include "../rt_runtime.h"

struct android_app;

namespace edge::android {
	class AndroidRuntimeContext;

	class AndroidRuntime final : public Runtime
	{
	public:
		AndroidRuntime(const IRuntimeContext& context);
		virtual ~AndroidRuntime() = default;

		int32_t initialize() override;

		void terminate(int32_t code) override;

		void process_input_events();

		void set_surface_ready();

		android_app* get_android_app();
		android_app const* get_android_app() const;

	protected:
		virtual void create_window(const Window::Properties& properties) override;
	private:
		const AndroidRuntimeContext* ctx{ nullptr };

		bool surface_ready{ false };
	};
}