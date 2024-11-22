#include <engine/EngineApplication.h>

//#include <integrations/imgui/DebugSystem.h>

#include <util/Logger.h>
#include <engine/rendering/Rendering.h>
#include <world/Scene.h>

#include <engine/ui/windows/HierarchyWindow.h>
#include <engine/ui/windows/InspectorWindow.h>
#include <engine/ui/windows/PostProcessWindow.h>

#include <tracy/Tracy.hpp>

namespace engine
{
	EngineApplication::EngineApplication()
	{
#ifdef PLATFORM_ANDROID
		auto logger = spdlog::android_logger_mt(name.c_str(), name.c_str());
#else
		auto logger = spdlog::stdout_color_mt("vulkan_app");
#endif

		logger->set_pattern(LOGGER_FORMAT);
		logger->set_level(spdlog::level::trace);

		//spdlog::register_logger(logger);
		spdlog::set_default_logger(logger);

		log_info("Starting app: {}", name);
	}

	EngineApplication::~EngineApplication()
	{

	}

	void EngineApplication::OnInputEvent(const InputEvent& input_event)
	{
		//m_pInputMapper->OnInputEvent(input_event);
	}

	void EngineApplication::Finish()
	{
	}

	bool EngineApplication::OnResize(uint32_t width, uint32_t height)
	{
		//m_pRenderInterface->UpdateSurfaceSize(width, height);
		return true;
	}

	bool EngineApplication::Prepare(const ApplicationOptions& options)
	{
		app_windows.emplace_back(std::make_unique<HierarchyWindow>("Hierarchy"));
		app_windows.emplace_back(std::make_unique<InspectorWindow>("Inspector"));
		app_windows.emplace_back(std::make_unique<PostProcessWindow>("PostProcess"));

		renderer = std::make_unique<render::Renderer>(options.window, options.virtual_filesystem);

		scene = std::make_unique<world::Scene>(options.virtual_filesystem, *renderer);
		//scene = std::make_unique<world::Scene>(vfs.get());
		scene->Load("/root/meshes/sponza/Sponza.gltf");
		//scene->Load("/root/meshes/helmet/DamagedHelmet.gltf");

		//render::FRenderInterfaceCreateInfo rhi_ci{};
		//rhi_ci.render_api = render::ERenderApi::eVulkan11;
		//rhi_ci.window = options.window;
		//
		//m_pRenderInterface->PreInitialize(rhi_ci);
		//m_pInputMapper->Create();
		//m_pSystemManager->PreInitialize();
		//// Pre initialization zone END
		//
		//// Initialization zone BEGIN
		//m_pSystemManager->Initialize();
		//m_pRenderInterface->Initialize();
		//// Initialization zone END
		//
		////m_pScene->load("/root/reflections/reflections.gltf");
		////m_pScene->load("/root/helmet/DamagedHelmet.gltf");
		//m_pScene->Load("/root/meshes/sponza/Sponza.gltf");
		////m_pScene->load("/root/meshes/san_miguel/low_poly.gltf");
		////m_pScene->MakeGenerateScene("/root/meshes/cube/cube.gltf");
		////m_pScene->makeTestScene();
		////m_pScene->load("/root/chess/chess.gltf");
		////m_pScene->load("/root/toadman/round1.gltf");
		////m_pScene->load("/root/Landscape/landscape.gltf");
		//
		//// Post initialization zone BEGIN
		//m_pRenderInterface->PostInitialize();
		//m_pSystemManager->PostInitialize();

		return true;
	}

	void EngineApplication::Update(float delta_time)
	{
		ZoneScoped;

		for (auto& window : app_windows)
			window->Draw(delta_time);

		scene->Update(delta_time);

		renderer->Render(delta_time);
		//// Pre update zone BEGIN
		//m_pSystemManager->PreUpdate(delta_time);
		//m_pRenderInterface->PreUpdate(delta_time);
		//// Pre update zone END
		//
		//// Update zone BEGIN
		//m_pScene->Update(delta_time);
		//m_pInputMapper->Update(delta_time);
		//m_pSystemManager->Update(delta_time);
		//m_pRenderInterface->Update(delta_time);
		//// Update zone END
		//
		//// Post update zone BEGIN
		//m_pSystemManager->PostUpdate(delta_time);
		//m_pRenderInterface->PostUpdate(delta_time);
		//// Post update zone EMD
		//
		//// Render zone BEGIN
		//m_pSystemManager->Render();
		//m_pRenderInterface->Render();
		//// Render zone END
	}
}