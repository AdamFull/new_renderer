#pragma once

#include "IAudioFile.h"

namespace e2d::audio {
	class AudioFile : public IAudioFile {
	public:
		virtual ~AudioFile() override = default;

		auto get_format() const noexcept -> AudioFormat const& final override;
		auto get_total_samples() const noexcept -> uint32_t final override;
		auto get_samples_done() const noexcept -> uint32_t final override;
	protected:
		AudioFormat format_;
		uint32_t total_samples_{ 0u };
		uint32_t samples_processed_{ 0u };

		std::span<const uint8_t> file_bytes_{};
	};
}