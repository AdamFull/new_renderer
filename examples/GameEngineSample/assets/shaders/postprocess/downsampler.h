#pragma once

namespace downsampler {
	struct PushConstant {
		float filter_radius;
		uint32_t current_mip;
	};
}