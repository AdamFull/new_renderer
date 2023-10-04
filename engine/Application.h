#pragma once

#include <memory>
#include <interface/WindowInterface.h>

namespace engine
{
	class CApplication
	{
	public:
	private:
		std::unique_ptr<IWindowInterface> window_handle{};
	};
}