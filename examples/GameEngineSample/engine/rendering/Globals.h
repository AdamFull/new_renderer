#pragma once

#include "../interface/render/PostProcessStructures.h"

namespace engine::render {
	extern PostProcessData postprocess;
	extern uint32_t main_camera_index;

	extern bool freeze_culling;
	extern bool is_culling_freezed;
	extern bool enable_frustum_culling;
	extern bool enable_occlusion_culling;
}