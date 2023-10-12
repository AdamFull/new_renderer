#pragma once

#include <memory>
#include <interface/WindowInterface.h>
#include <interface/RenderInterface.h>
#include <interface/filesystem/VirtualFileSystemInterface.h>

namespace engine
{
	class CApplication
	{
	public:
		void create();
		void loop();

		size_t loadTexture(const std::string& path);
	private:
		std::unique_ptr<IWindowInterface> window_handle{};
		std::unique_ptr<IRenderInterface> render_interface{};
		std::unique_ptr<filesystem::IVirtualFileSystemInterface> vfs{};
	};
}