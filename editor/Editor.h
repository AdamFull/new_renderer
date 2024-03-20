#pragma once

#include <engine/interface/WindowInterface.h>
#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/InputMapper.h>

namespace editor
{
	class CEditor
	{
	public:
		void create();
		void loop();
	private:
		stl::unique_ptr<engine::IWindowInterface> m_pWindowHandle{};
		stl::unique_ptr<engine::CInputMapper> m_pInputMapper{};
		stl::unique_ptr<engine::filesystem::IVirtualFileSystemInterface> m_pVirtualFileSystem{};
	};
}