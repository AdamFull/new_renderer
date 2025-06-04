#include "App.h"

#include <edge/util/Logger.h>
#include <edge/util/utl_profiler.h>
#include <edge/gfx/gfx_profiler.h>

namespace e2d {
	App::App() {
		edge::LoggerCreateInfo logger_create_info{};
		logger_create_info.logger_name = "E2D";
		edge::CreateLogger(logger_create_info);

		EDGE_LOGI("Starting app: {}", name);
	}

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
		return true;
	}

	void App::update(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		EDGE_GFX_PROFILER_DRAW_UI();

		EDGE_GFX_PROFILER_FRAME_MARK();
	}
}