#pragma once

#include <span>

#include "Common.h"

namespace e2d::audio {
	class IAudioFile {
	public:
		virtual ~IAudioFile() = default;

		virtual auto get_format() const noexcept -> AudioFormat const& = 0;
		virtual auto get_total_samples() const noexcept -> uint32_t = 0;
		virtual auto get_samples_done() const noexcept -> uint32_t = 0;
		virtual auto read_samples(std::span<uint8_t> out, const AudioFormat& out_fmt) -> uint32_t = 0;
	};
}