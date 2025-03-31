#include "Edge.h"

#if defined(PLATFORM_WINDOWS)
#include "runtime/win64/rt_windows_runtime_context.h"
#include "runtime/win64/rt_windows_runtime.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	auto context = std::make_unique<edge::win64::WindowsRuntimeContext>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	return runtime_main(*context);
}
#elif defined(PLATFORM_ANDROID)
#include "runtime/android/rt_android_runtime_context.h"
#include "runtime/android/rt_android_runtime.h"

extern "C" void android_main(android_app* state)
{
	auto context = std::make_unique<edge::android::AndroidRuntimeContext>(state);
	runtime_main(*context);
}

#else
# include <stdexcept>
#error "Platform not supported"

int main(int argc, char* argv[])
{
	EDGE_LOGE("[Runtime] platform not supported");
}
#endif

namespace edge
{
	std::unique_ptr<Runtime> create_application_context(IRuntimeContext& context)
	{
#if defined(PLATFORM_WINDOWS)
		return std::make_unique<win64::WindowsRuntime>(context);
#elif defined(PLATFORM_ANDROID)
		return std::make_unique<android::AndroidRuntime>(context);
#else
#	error "Platform not supported"
		return nullptr;
#endif
	}
}