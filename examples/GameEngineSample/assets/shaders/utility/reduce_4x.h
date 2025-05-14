#pragma once

#include "../include/interop.h"

namespace reduce {
	struct PushConstants {
		uint32_t mip_count;
		uint32_t mip_level;
		float2 texel_size;
	};
}