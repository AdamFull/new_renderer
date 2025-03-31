#pragma once

#include "../rt_runtime_context.h"

struct android_app;

namespace edge::android
{
	class AndroidRuntimeContext final : public IRuntimeContext {
	public:
		AndroidRuntimeContext(android_app* app);
		virtual ~AndroidRuntimeContext() = default;

		android_app* get_app();
		const android_app* get_app() const;
	private:
		android_app* app{ nullptr };
	};
}