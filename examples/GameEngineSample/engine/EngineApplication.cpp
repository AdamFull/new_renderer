#include "EngineApplication.h"

//#include <integrations/imgui/DebugSystem.h>

#include <edge/fs/native/fs_native_filesystem.h>
#include <edge/fs/zip/fs_zip_filesystem.h>
#include <edge/fs/fs_helper.h>
#include <edge/fs/fs_virtual_filesystem.h>

#include <edge/runtime/rt_runtime.h>
#include <edge/util/Logger.h>

#include "rendering/Rendering.h"

#include "ui/windows/HierarchyWindow.h"
#include "ui/windows/InspectorWindow.h"
#include "ui/windows/PostProcessWindow.h"

#include "InputManager.h"

//#include <imdbg.h>

#include <edge/util/utl_profiler.h>
#include <edge/gfx/gfx_profiler.h>

namespace engine {
	void EngineApplication::on_input_event(const edge::InputEvent& input_event) {
		InputManager::DispatchInputEvent(input_event);
	}

	void EngineApplication::finish() {
		//ImDbg::DestroyContext();
	}

	bool EngineApplication::on_resize(uint32_t width, uint32_t height) {
		//m_pRenderInterface->UpdateSurfaceSize(width, height);
		return true;
	}

	bool EngineApplication::prepare(const edge::ApplicationOptions& options) {
		bool result = edge::Application::prepare(options);

		auto& app_path = options.runtime->get_app_path();
		auto app_dir_path = edge::fs::parent_path(app_path);

		// Mount default engine root filesystem
#if EDGE_PLATFORM_ANDROID
		// Open base.apk path
		edge::fs::mount("/internal_res", new edge::fs::ZipFilesystem(app_path));
#else
		std::array<char, 512> buffer{};
		auto engine_internal_resources_path = edge::fs::path_append(app_dir_path, "internal.pak", buffer);
		edge::fs::mount("/internal_res", new edge::fs::ZipFilesystem(engine_internal_resources_path));
#endif
		
		edge::fs::mount("/internal", new edge::fs::NativeFilesystem(app_dir_path));

		ui = std::make_unique<EngineUI>(*options.window);

		app_windows.emplace_back(std::make_unique<HierarchyWindow>("Hierarchy"));
		app_windows.emplace_back(std::make_unique<InspectorWindow>("Inspector"));
		app_windows.emplace_back(std::make_unique<PostProcessWindow>("PostProcess"));

		renderer = std::make_unique<render::Renderer>(options.window);

		scene = std::make_unique<world::Scene>(*renderer);
		//scene = std::make_unique<world::Scene>(vfs.get());
		scene->Load("/internal/meshes/sponza/Sponza.gltf");
		//scene->Load("/root/meshes/CesiumMan/CesiumMan.gltf");
		
		return result;
	}

	void EngineApplication::update(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		ui->NewFrame(delta_time);

		EDGE_GFX_PROFILER_DRAW_UI();

		for (auto& window : app_windows) {
			window->Draw(delta_time);
		}

		scene->Update(delta_time);

		renderer->render(delta_time);

		EDGE_GFX_PROFILER_FRAME_MARK();
	}
}