#include "AudioFile.h"

namespace e2d::audio {
	auto AudioFile::get_format() const noexcept -> AudioFormat const& {
		return format_;
	}

	auto AudioFile::get_total_samples() const noexcept -> uint32_t {
		return total_samples_;
	}

	auto AudioFile::get_samples_done() const noexcept -> uint32_t {
		return samples_processed_;
	}
}