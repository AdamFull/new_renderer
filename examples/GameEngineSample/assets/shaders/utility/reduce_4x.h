#pragma once

#include "../include/interop.h"

namespace reduce {
	static const uint32_t k_mip_per_pass = 4u;

	struct PushConstants {
		uint32_t mip_count;
		uint32_t mip_level;
		float2 texel_size;
	};
}