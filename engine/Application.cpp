#include <Application.h>

#include <platform/PlatformSpecific.h>
#include <render/RenderInterface.h>

#include <filesystem/VirtualFileSystemManager.h>
#include <filesystem/native/NativeFileSystem.h>
#include <filesystem/vfs_helper.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <interface/SharedStructure.h>


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

	FRenderInterfaceCreateInfo rhi_ci{};
	rhi_ci.render_api = ERenderApi::eVulkan13;
	m_pWindowHandle->getWindowSize(rhi_ci.initialWidth, rhi_ci.initialHeight);
	get_platform_specific_window_data(rhi_ci, m_pWindowHandle);

	m_pRenderInterface = std::make_unique<render::CRenderInterface>(m_pVirtualFileSystem.get());
	m_pRenderInterface->create(rhi_ci);

	m_pScene = std::make_unique<world::CScene>(m_pVirtualFileSystem.get(), m_pRenderInterface.get());
	m_pScene->load("/root/reflections/reflections.gltf");

	// TODO: incorrect stb image loading. I guess it requires to generate copy info
	auto ttt = m_pRenderInterface->createTexture("/root/cavern-deposits_albedo.ktx2");
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

		m_pScene->update(dt);
		// TODO: render thread should signal game thread about finish rendering
		m_pRenderInterface->render(dt);

		dt = sw.stop<float>();
	}
}