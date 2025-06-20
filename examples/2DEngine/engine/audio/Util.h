#pragma once

#include "Common.h"

namespace e2d::audio {
	auto get_internal_format(const AudioFormat& fmt) -> uint8_t;
	auto get_samples_per_channel(const AudioFormat& fmt, size_t buffer_size) -> size_t;
}