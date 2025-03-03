#pragma once

#include "../RuntimeContext.h"

struct android_app;

namespace edge
{
	namespace android
	{
		class AndroidRuntimeContext final : public IRuntimeContext
		{
		public:
			AndroidRuntimeContext(android_app* app);
			virtual ~AndroidRuntimeContext() = default;

			android_app* GetApp();
			const android_app* GetApp() const;
		private:
			android_app* app{ nullptr };
		};
	}
}