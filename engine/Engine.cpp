#include "Engine.h"

#include <engine/platform/PlatformSpecific.h>
#include <engine/render/RenderInterface.h>

#include <engine/filesystem/VirtualFileSystemManager.h>
#include <engine/filesystem/native/NativeFileSystem.h>
#include <engine/filesystem/vfs_helper.h>

#include <integrations/imgui/ImGuiSystem.h>
#include <integrations/imgui/DebugSystem.h>

using namespace engine;
using namespace engine::render;

void get_platform_specific_window_data(FRenderInterfaceCreateInfo& rhi_ci, const std::unique_ptr<IWindowInterface>& window)
{
#if defined(PLATFORM_WIN32)
	auto platform_window = utl::cast<platform::CWindowGLFW*>(window.get());
	platform_window->GetPlatformData(rhi_ci.hinstance, rhi_ci.hwnd);
#endif
}

CEngine::CEngine()
{
	m_pSystemManager = std::make_unique<CEngineSystemManager>();
}

void CEngine::SetApplication(std::unique_ptr<IApplication>&& app)
{
	m_pApplication = std::move(app);
}

void CEngine::Initialize()
{
	m_pVirtualFileSystem = std::make_unique<filesystem::CVirtualFileSystemManager>();
	m_pVirtualFileSystem->Mount("/root", std::make_unique<filesystem::CNativeFileSystem>("./"));

	m_pRenderInterface = std::make_unique<render::CRenderInterface>(m_pVirtualFileSystem.get());

	m_pInputMapper = std::make_unique<CInputMapper>();

	//m_pSystemManager->RegisterSystem<CImGuiEngineSystem>(m_pRenderInterface.get());
	m_pSystemManager->RegisterSystem<CDebugDrawEngineSystem>(m_pRenderInterface.get());

	FWindowCreateInfo window_ci{};
	window_ci.name = "Vulkan App";
	window_ci.flags = WINDOW_RESIZABLE_FLAG_BIT;
	window_ci.width = 2560;
	window_ci.height = 1440;

	m_pWindowHandle = make_platform_window(window_ci);

	// Pre initialization zone BEGIN
	FRenderInterfaceCreateInfo rhi_ci{};
	rhi_ci.render_api = ERenderApi::eVulkan11;
	m_pWindowHandle->GetWindowSize(rhi_ci.initialWidth, rhi_ci.initialHeight);
	get_platform_specific_window_data(rhi_ci, m_pWindowHandle);

	m_pRenderInterface->PreInitialize(rhi_ci);
	m_pInputMapper->Create();
	m_pSystemManager->PreInitialize();
	m_pApplication->PreInitialize();
	// Pre initialization zone END

	// Initialization zone BEGIN
	m_pRenderInterface->Initialize();
	m_pSystemManager->Initialize();
	m_pApplication->Initialize();
	// Initialization zone END

	m_pScene = std::make_unique<world::CScene>(m_pVirtualFileSystem.get(), m_pRenderInterface.get());
	//m_pScene->load("/root/reflections/reflections.gltf");
	//m_pScene->load("/root/helmet/DamagedHelmet.gltf");
	m_pScene->Load("/root/meshes/sponza/Sponza.gltf");
	//m_pScene->load("/root/meshes/san_miguel/low_poly.gltf");
	//m_pScene->MakeGenerateScene("/root/meshes/cube/cube.gltf");
	//m_pScene->makeTestScene();
	//m_pScene->load("/root/chess/chess.gltf");
	//m_pScene->load("/root/toadman/round1.gltf");
	//m_pScene->load("/root/Landscape/landscape.gltf");

	// Post initialization zone BEGIN
	m_pRenderInterface->PostInitialize();
	m_pSystemManager->PostInitialize();
	m_pApplication->PostInitialize();
	// Post initialization zone END

	Loop();
}

void CEngine::Loop()
{
	utl::utils::stopwatch sw{};
	f32 dt{ 0.f };

	while (m_pWindowHandle->Process())
	{
		if (m_pWindowHandle->IsResized())
		{
			// Wait while window is minimized
			while (m_pWindowHandle->IsMinimized())
			{
				if (!m_pWindowHandle->Process())
					return;
			}

			i32 window_width{}, window_height{};
			m_pWindowHandle->GetWindowSize(window_width, window_height);
			m_pRenderInterface->UpdateSurfaceSize(window_width, window_height);
		}

		// Pre update zone BEGIN
		m_pRenderInterface->PreUpdate(dt);
		m_pSystemManager->PreUpdate(dt);
		m_pApplication->PreUpdate(dt);
		// Pre update zone END

		// Update zone BEGIN
		m_pScene->Update(dt);
		m_pInputMapper->Update(dt);
		m_pRenderInterface->Update(dt);
		m_pSystemManager->Update(dt);
		m_pApplication->Update(dt);
		// Update zone END

		// Post update zone BEGIN
		m_pRenderInterface->PostUpdate(dt);
		m_pSystemManager->PostUpdate(dt);
		m_pApplication->PostUpdate(dt);
		// Post update zone EMD

		// Pre render zone BEGIN
		m_pRenderInterface->PreRender();
		m_pSystemManager->PreRender();
		m_pApplication->PreRender();
		// Pre render zone END

		// Render zone BEGIN
		m_pRenderInterface->Render();
		m_pSystemManager->Render();
		m_pApplication->Render();
		// Render zone END

		// Post render zone BEGIN
		m_pRenderInterface->PostRender();
		m_pSystemManager->PostRender();
		m_pApplication->PostRender();
		// Post render zone END

		//TRACY_PROFILE_FRAMEMARKER();

		dt = sw.stop<f32>();

		auto& render_context = m_pRenderInterface->GetRenderContext();
		auto& render_stats = render_context->GetRenderFrameStats();

		cpuCounter.update(dt);
		gpuCounter.update(render_stats.gpuFrameTime);

		UpdateTitle();
	}
}

void CEngine::UpdateTitle()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto& render_stats = render_context->GetRenderFrameStats();

	float used_gpu_memory = utl::cast<float>(render_stats.usedGPUMemory) / 1024.f / 1024.f / 1024.f;
	float total_gpu_memory = utl::cast<float>(render_stats.totalGPUMemory) / 1024.f / 1024.f / 1024.f;

	auto winname = std::format("Vulkan App {} fps ({:.2f} ms); {} gpu fps ({:.2f} ms); gpu memory ({:.2f}/{:.2f} Gb)", cpuCounter.fps(), cpuCounter.delta_time() * 1000.f, gpuCounter.fps(), gpuCounter.delta_time() * 1000.f, used_gpu_memory, total_gpu_memory);
	m_pWindowHandle->SetTitle(winname);
}