#include "AndroidNativeSurface.h"

namespace edge
{
	namespace android
	{
		AndroidNativeSurface::AndroidNativeSurface(ANativeWindow*& window) :
			handle{ window }
		{
		}

		ANativeWindow*& AndroidNativeSurface::GetHandle()
		{
			return handle;
		}
	}
}