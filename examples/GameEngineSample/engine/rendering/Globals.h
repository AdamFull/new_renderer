#pragma once

#include "../../assets/shaders/include/interop.h"
#include "../interface/render/PostProcessStructures.h"

namespace engine::render {
	extern PostProcessData postprocess;
	extern uint32_t main_camera_index;

	extern interop::CullingFlags culling_flags;
	extern bool is_culling_freezed;
}