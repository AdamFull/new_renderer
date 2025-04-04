#pragma once

#include "../rt_runtime.h"

namespace edge::win64 {
	class WindowsRuntime final : public Runtime {
	public:
		WindowsRuntime(const IRuntimeContext& context);
		virtual ~WindowsRuntime() = default;

        int32_t initialize() override;

	protected:
		virtual void create_window(const Window::Properties& properties) override;
	};
}