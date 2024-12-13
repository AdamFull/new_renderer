#pragma once

#include "../Runtime.h"

namespace edge
{
	namespace win32
	{
		class WindowsRuntime final : public Runtime
		{
		public:
			WindowsRuntime(const RuntimeContext& context);
			virtual ~WindowsRuntime() = default;

		protected:
			virtual void CreateWindow(const Window::Properties& properties) override;
		};
	}
}