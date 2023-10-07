#pragma once

#include <memory>
#include <interface/WindowInterface.h>
#include <interface/RenderInterface.h>

namespace engine
{
	class CApplication
	{
	public:
		void create();
		void loop();
	private:
		std::unique_ptr<IWindowInterface> window_handle{};
		std::unique_ptr<IRenderInterface> render_interface{};
	};
}