#pragma once

#include <memory>
#include <engine/interface/WindowInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/scripting/ScriptEngine.h>
#include <engine/world/Scene.h>

#include "InputMapper.h"

namespace engine
{
	class CApplication
	{
	public:
		void create();
		void loop();

	private:
		std::unique_ptr<IWindowInterface> m_pWindowHandle{};
		std::unique_ptr<CInputMapper> m_pInputMapper{};
		std::unique_ptr<render::IRenderInterface> m_pRenderInterface{};
		std::unique_ptr<filesystem::IVirtualFileSystemInterface> m_pVirtualFileSystem{};
		std::unique_ptr<scripting::CScriptEngine> m_pScriptEngine{};
		std::unique_ptr<world::CScene> m_pScene{};
	};
}