#pragma once

#include "../NativeSurface.h"

namespace edge
{
	namespace android
	{
		class AndroidNativeSurface : public NativeSurface
		{
		public:
			AndroidNativeSurface(ANativeWindow*& window);
			~AndroidNativeSurface() = default;

			ANativeWindow*& GetHandle();
		private:
			ANativeWindow*& handle;
		};
	}
}