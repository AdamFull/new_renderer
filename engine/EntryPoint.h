#pragma once

#include <memory>

#include "runtime/RuntimeContext.h"

int RuntimeMain(const engine::RuntimeContext&);

#if defined(PLATFORM_WINDOWS)
#include "runtime/windows/WindowsRuntimeContext.h"
#include "runtime/windows/WindowsRuntime.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	auto context = std::make_unique<engine::win32::WindowsRuntimeContext>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
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