#pragma once

#include <reactive/function.h>
#include <engine/interface/WindowInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/scripting/ScriptEngine.h>
#include <engine/world/Scene.h>
#include <interface/EngineSystem.h>
#include <interface/Application.h>
#include "InputMapper.h"

#include <utils/stopwatch.h>

namespace engine
{
	class CEngine
	{
	public:
		void SetApplication(std::unique_ptr<IApplication>&& app);
		void Initialize();
	private:
		void Loop();
		void UpdateTitle();
	private:
		std::unique_ptr<IApplication> m_pApplication{};
		std::unique_ptr<IWindowInterface> m_pWindowHandle{};
		std::unique_ptr<CInputMapper> m_pInputMapper{};
		std::unique_ptr<render::IRenderInterface> m_pRenderInterface{};
		std::unique_ptr<filesystem::IVirtualFileSystemInterface> m_pVirtualFileSystem{};
		std::unique_ptr<world::CScene> m_pScene{};

		utl::utils::fps_counter32 cpuCounter{};
		utl::utils::fps_counter32 gpuCounter{};
	};
}