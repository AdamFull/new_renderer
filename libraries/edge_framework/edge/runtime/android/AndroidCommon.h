#pragma once

struct android_app;

namespace edge
{
	namespace android
	{
		bool ProcessAndroidEvents(android_app* app);
	}
}