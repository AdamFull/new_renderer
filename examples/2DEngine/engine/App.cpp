#include "App.h"

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

		RendererCreateInfo initial_create_info{};
		renderer = Renderer::construct(initial_create_info);

		return result;
	}

	void App::update(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		EDGE_GFX_PROFILER_DRAW_UI();

		EDGE_GFX_PROFILER_FRAME_MARK();
	}
}