#pragma once

#include <memory>
#include <interface/WindowInterface.h>
#include <interface/RenderInterface.h>
#include <interface/filesystem/VirtualFileSystemInterface.h>
#include <scripting/ScriptEngine.h>

namespace engine
{
	class CApplication
	{
	public:
		void create();
		void loop();

	private:
		std::unique_ptr<IWindowInterface> m_pWindowHandle{};
		std::unique_ptr<render::IRenderInterface> m_pRenderInterface{};
		std::unique_ptr<filesystem::IVirtualFileSystemInterface> m_pVirtualFileSystem{};
		std::unique_ptr<scripting::CScriptEngine> m_pScriptEngine{};
	};
}