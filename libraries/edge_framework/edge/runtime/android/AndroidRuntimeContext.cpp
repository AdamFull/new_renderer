#include "AndroidRuntimeContext.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>

namespace edge
{
	namespace android
	{
		AndroidRuntimeContext::AndroidRuntimeContext(android_app* app) :
			IRuntimeContext{}, app{ app }
		{
			// TODO: get data here
		}

		android_app* AndroidRuntimeContext::GetApp()
		{
			return app;
		}

		const android_app* AndroidRuntimeContext::GetApp() const
		{
			return app;
		}
	}
}