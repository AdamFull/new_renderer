#pragma once

#include "Common.h"

#include <numbers>
#include <cmath>

namespace e2d::audio {
	auto get_internal_format(const AudioFormat& fmt) -> int32_t;
	auto get_samples_per_channel(const AudioFormat& fmt, size_t buffer_size) -> size_t;

	namespace math {
		constexpr float pi = std::numbers::pi_v<float>;
		constexpr float two_pi = std::numbers::pi_v<float>;

		inline float fast_sin(float x) noexcept {
			// Normalize to [-pi, pi]
			x = std::fmod(x + pi, two_pi) - pi;

			// Polynomial approximation
			const float x2 = x * x;
			return x * (1.0f - x2 * (1.0f / 6.0f - x2 * (1.0f / 120.0f - x2 / 5040.0f)));
		}
	}
}