#include "rt_application.h"

#include <cassert>

#include <edge/util/Logger.h>

#include <edge/gfx/gfx_resource_loader.h>
#include <edge/gfx/gfx_context.h>

namespace edge {
	Application::~Application() {
		if (resource_uploader_initialized) {
			edge::gfx::shutdown_resource_uploader();
		}

		if (graphics_initialized) {
			edge::gfx::shutdown_context();
		}
	}

	auto Application::prepare(const ApplicationOptions& options) -> bool {
		assert(options.window != nullptr && "Window must be valid");

		// Initialize application logger
		edge::LoggerCreateInfo logger_create_info{};
		logger_create_info.logger_name = name;
		edge::CreateLogger(logger_create_info);
		EDGE_LOGI("Starting app: {}", name);

		// Init graphics if needed
		if (options.features.use_graphics) {
			edge::gfx::ContextCreateInfo gfx_create_info{};
			gfx_create_info.app_name = name.c_str();
			gfx_create_info.render_api = edge::gfx::RenderApi::eVulkan;
			gfx_create_info.window = options.window;
			gfx_create_info.require_features.ray_tracing = options.features.use_ray_tracing;
			edge::gfx::make_context(gfx_create_info);
			graphics_initialized = true;
		}

		if (options.features.use_graphics && options.features.use_uploader) {
			edge::gfx::ResourceUpdaterCreateInfo resource_updater_init{};
			edge::gfx::make_resource_uploader(resource_updater_init);
			resource_uploader_initialized = true;
		}
		
		window = options.window;

		return true;
	}
}