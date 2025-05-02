#pragma once

#include "../include/interop.h"

namespace comp_histogram {
	struct PushConstant {
		TYPE_PTR(uint32_t) histogram;

		float min_log_lum;
		float log_lum_range;
	};
}

namespace comp_brightness {
	struct PushConstant {
		TYPE_PTR(uint32_t) histogram;

		float min_log_lum;
		float log_lum_range;
		float time_coeff;
		float num_pixels_x;
		float num_pixels_y;
		float _pad0;
	};
}