#pragma once

#include <cstdint>

namespace e2d::audio {
	struct AudioFormat {
		uint32_t sample_rate{ 44100 };
		uint16_t channels{ 2 };
		uint16_t bit_per_sample{ 16 };
	};
}