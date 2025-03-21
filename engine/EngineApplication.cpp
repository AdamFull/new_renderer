#include "EngineApplication.h"

//#include <integrations/imgui/DebugSystem.h>

#include <edge/util/Logger.h>

#include "rendering/Rendering.h"
#include "world/Scene.h"

#include "ui/windows/HierarchyWindow.h"
#include "ui/windows/InspectorWindow.h"
#include "ui/windows/PostProcessWindow.h"

#include "InputManager.h"

#include <imdbg.h>

#include <edge/util/utl_profiler.h>

namespace engine
{
	EngineApplication::EngineApplication()
	{
		edge::LoggerCreateInfo logger_create_info{};
		logger_create_info.logger_name = "Vulkan app";
		edge::CreateLogger(logger_create_info);

		EDGE_LOGI("Starting app: {}", name);
	}

	EngineApplication::~EngineApplication()
	{

	}

	void EngineApplication::on_input_event(const edge::InputEvent& input_event)
	{
		InputManager::DispatchInputEvent(input_event);
	}

	void EngineApplication::finish()
	{
		ImDbg::DestroyContext();
	}

	bool EngineApplication::on_resize(uint32_t width, uint32_t height)
	{
		//m_pRenderInterface->UpdateSurfaceSize(width, height);
		return true;
	}

	bool EngineApplication::prepare(const edge::ApplicationOptions& options)
	{
		ImDbg::create_context();

		ui = std::make_unique<EngineUI>(*options.window);

		app_windows.emplace_back(std::make_unique<HierarchyWindow>("Hierarchy"));
		app_windows.emplace_back(std::make_unique<InspectorWindow>("Inspector"));
		app_windows.emplace_back(std::make_unique<PostProcessWindow>("PostProcess"));

		renderer = std::make_unique<render::Renderer>(options.window);

		scene = std::make_unique<world::Scene>(*renderer);
		//scene = std::make_unique<world::Scene>(vfs.get());
		scene->Load("/root/meshes/sponza/Sponza.gltf");
		//scene->Load("/root/meshes/CesiumMan/CesiumMan.gltf");
		
		return true;
	}

	void EngineApplication::update(float delta_time)
	{
		EDGE_PROFILE_ZONE_SCOPED;

		ImDbg::NewFrame();

		ui->NewFrame(delta_time);

		for (auto& window : app_windows)
			window->Draw(delta_time);

		scene->Update(delta_time);

		ImDbg::end_frame();



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