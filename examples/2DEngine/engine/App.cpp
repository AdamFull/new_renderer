#include "App.h"

#include <edge/fs/fs_helper.h>
#include <edge/fs/zip/fs_zip_filesystem.h>
#include <edge/fs/native/fs_native_filesystem.h>

#include <edge/runtime/rt_runtime.h>
#include <edge/util/Logger.h>
#include <edge/util/utl_profiler.h>
#include <edge/gfx/gfx_profiler.h>

#include <edge/gfx/gfx_resource_loader.h>
#include <edge/gfx/gfx_context.h>

#include "render/Renderer.h"

namespace e2d {
	App::~App() {
	}

	void App::on_input_event(const edge::InputEvent& input_event) {
	}

	void App::finish() {
	}

	bool App::on_resize(uint32_t width, uint32_t height) {
		return true;
	}

	bool App::prepare(const edge::ApplicationOptions& options) {
		auto result = edge::Application::prepare(options);

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

		RendererCreateInfo initial_create_info{};
		initial_create_info.window = options.window;
		renderer = Renderer::construct(initial_create_info);

		return result;
	}

	void App::update(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		renderer->begin_frame(delta_time);

		EDGE_GFX_PROFILER_DRAW_UI();

		renderer->end_frame(delta_time);
		EDGE_GFX_PROFILER_FRAME_MARK();
	}
}