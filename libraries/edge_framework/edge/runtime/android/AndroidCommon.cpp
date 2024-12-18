#include "AndroidCommon.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>

namespace edge
{
	namespace android
	{
		bool ProcessAndroidEvents(android_app* app)
		{
			android_poll_source* source;

			int ident;
			int events;

			while ((ident = ALooper_pollOnce(0, nullptr, &events, (void**)&source)) > ALOOPER_POLL_TIMEOUT)
			{
				if (source)
				{
					source->process(app, source);
				}

				if (app->destroyRequested != 0)
				{
					return false;
				}
			}
			if (app->userData)
			{
				auto platform = reinterpret_cast<AndroidPlatform*>(app->userData);
				platform->process_android_input_events();
			}
			return true;
		}
	}
}