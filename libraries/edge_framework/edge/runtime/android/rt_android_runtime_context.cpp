#include "rt_android_runtime_context.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>

namespace edge::android {
	AndroidRuntimeContext::AndroidRuntimeContext(android_app* app) :
		IRuntimeContext{}, app{ app } {
		// TODO: get data here
	}

	android_app* AndroidRuntimeContext::get_app() {
		return app;
	}

	const android_app* AndroidRuntimeContext::get_app() const {
		return app;
	}
}