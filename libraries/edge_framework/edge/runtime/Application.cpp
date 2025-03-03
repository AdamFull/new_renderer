#include "Application.h"

#include <cassert>

namespace edge
{
	auto Application::Prepare(const ApplicationOptions& options) -> bool
	{
		assert(options.window != nullptr && "Window must be valid");

		//auto& _debug_info = get_debug_info();
		//_debug_info.insert<field::MinMax, float>("fps", fps);
		//_debug_info.insert<field::MinMax, float>("frame_time", frame_time);

		lock_simulation_speed = options.benchmark_enabled;
		window = options.window;

		return true;
	}
}