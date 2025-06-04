#include "App.h"

#include <edge/util/Logger.h>
#include <edge/util/utl_profiler.h>
#include <edge/gfx/gfx_profiler.h>

#include <edge/gfx/gfx_resource_loader.h>
#include <edge/gfx/gfx_context.h>

namespace e2d {
	void App::on_input_event(const edge::InputEvent& input_event) {
	}

	void App::finish() {
	}

	bool App::on_resize(uint32_t width, uint32_t height) {
		return true;
	}

	bool App::prepare(const edge::ApplicationOptions& options) {
		return edge::Application::prepare(options);
	}

	void App::update(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		EDGE_GFX_PROFILER_DRAW_UI();

		EDGE_GFX_PROFILER_FRAME_MARK();
	}
}