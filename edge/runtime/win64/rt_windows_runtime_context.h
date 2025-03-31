#pragma once

#include "../rt_runtime_context.h"

#include <Windows.h>

namespace edge::win64 {
	class WindowsRuntimeContext final : public IRuntimeContext {
	public:
		WindowsRuntimeContext(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow);
		virtual ~WindowsRuntimeContext() = default;
	};
}