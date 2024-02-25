#pragma once

#include <standart_library.h>
#include <engine/interface/WindowInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/scripting/ScriptEngine.h>
#include <engine/world/Scene.h>

#include <utils/stopwatch.h>

#include "InputMapper.h"

namespace engine
{
	class CApplication
	{
	public:
		void create();
		void loop();
	private:
		void updateTitle();
	private:
		stl::unique_ptr<IWindowInterface> m_pWindowHandle{};
		stl::unique_ptr<CInputMapper> m_pInputMapper{};
		stl::unique_ptr<render::IRenderInterface> m_pRenderInterface{};
		stl::unique_ptr<filesystem::IVirtualFileSystemInterface> m_pVirtualFileSystem{};
		stl::unique_ptr<scripting::CScriptEngine> m_pScriptEngine{};
		stl::unique_ptr<world::CScene> m_pScene{};

		utl::utils::fps_counter32 cpuCounter{};
		utl::utils::fps_counter32 gpuCounter{};
	};
}