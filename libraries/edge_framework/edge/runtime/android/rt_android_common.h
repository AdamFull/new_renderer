#pragma once

struct android_app;

namespace edge::android {
	bool process_android_events(android_app* app);
}