#include "Application.h"

#include <engine/platform/PlatformSpecific.h>
#include <engine/render/RenderInterface.h>

#include <engine/filesystem/VirtualFileSystemManager.h>
#include <engine/filesystem/native/NativeFileSystem.h>
#include <engine/filesystem/vfs_helper.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <engine/interface/SharedStructure.h>

#include <engine/profiler/Profiler.h>

using namespace engine;
using namespace engine::render;

void get_platform_specific_window_data(FRenderInterfaceCreateInfo& rhi_ci, const std::unique_ptr<IWindowInterface>& window)
{
#if defined(PLATFORM_WIN32)
	auto platform_window = static_cast<platform::CWindowGLFW*>(window.get());
	platform_window->getPlatformData(rhi_ci.hinstance, rhi_ci.hwnd);
#endif
}

void CApplication::create()
{
	m_pVirtualFileSystem = std::make_unique<filesystem::CVirtualFileSystemManager>();

	m_pVirtualFileSystem->mount("/root", std::make_unique<filesystem::CNativeFileSystem>("./"));

	m_pScriptEngine = std::make_unique<scripting::CScriptEngine>();
	m_pScriptEngine->initialize();

	FWindowCreateInfo window_ci{};
	window_ci.name = "Vulkan App";
	window_ci.flags = WINDOW_RESIZABLE_FLAG_BIT;
	window_ci.width = 1920;
	window_ci.height = 1080;

	m_pWindowHandle = make_platform_window(window_ci);

	m_pInputMapper = std::make_unique<CInputMapper>();
	m_pInputMapper->create();

	FRenderInterfaceCreateInfo rhi_ci{};
	rhi_ci.render_api = ERenderApi::eVulkan11;
	m_pWindowHandle->getWindowSize(rhi_ci.initialWidth, rhi_ci.initialHeight);
	get_platform_specific_window_data(rhi_ci, m_pWindowHandle);

	m_pRenderInterface = std::make_unique<render::CRenderInterface>(m_pVirtualFileSystem.get());
	m_pRenderInterface->create(rhi_ci);

	m_pScene = std::make_unique<world::CScene>(m_pVirtualFileSystem.get(), m_pRenderInterface.get());
	//m_pScene->load("/root/reflections/reflections.gltf");
	//m_pScene->load("/root/helmet/DamagedHelmet.gltf");
	//m_pScene->load("/root/sponza/Sponza.gltf");
	m_pScene->load("/root/chess/chess.gltf");
	//m_pScene->load("/root/toadman/round1.gltf");
	//m_pScene->load("/root/Landscape/landscape.gltf");
}

void CApplication::loop()
{
	utl::stopwatch sw{};
	float dt{ 0.f };

	while (m_pWindowHandle->process())
	{
		if (m_pWindowHandle->isResized())
		{
			// Wait while window is minimized
			while (m_pWindowHandle->isMinimized())
			{
				if (!m_pWindowHandle->process())
					return;
			}

			int32_t window_width{}, window_height{};
			m_pWindowHandle->getWindowSize(window_width, window_height);
			m_pRenderInterface->updateSurfaceSize(window_width, window_height);
		}

		m_pInputMapper->update(dt);

		m_pScene->update(dt);
		// TODO: render thread should signal game thread about finish rendering
		m_pRenderInterface->render(dt);

		TRACY_PROFILE_FRAMEMARKER();

		dt = sw.stop<float>();
	}
}