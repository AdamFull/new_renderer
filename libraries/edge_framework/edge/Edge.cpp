#include "Edge.h"

#if defined(PLATFORM_WINDOWS)
#include "runtime/win64/WindowsRuntimeContext.h"
#include "runtime/win64/WindowsRuntime.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	auto context = std::make_unique<edge::win32::WindowsRuntimeContext>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	return RuntimeMain(*context);
}
#else
# include <stdexcept>
#error "Platform not supported"

int main(int argc, char* argv[])
{
	throw std::runtime_error{ "platform not supported" };
}
#endif

namespace edge
{
	std::unique_ptr<Runtime> CreateApplicationContext(const RuntimeContext& context)
	{
#if defined(PLATFORM_WINDOWS)
		return std::make_unique<win32::WindowsRuntime>(context);
#else
#	error "Platform not supported"
		return nullptr;
#endif
	}
}