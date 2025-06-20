#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include <dr_wav.h>

#include "IAudioFile.h"

namespace e2d::audio {
	class WavFile final : public IAudioFile {
	public:
		WavFile();
		virtual ~WavFile() = default;

		[[nodiscard]] static auto construct(std::string_view path) -> std::unique_ptr<IAudioFile>;

		auto get_format() const noexcept -> AudioFormat const& override;
		auto get_total_samples() const noexcept -> uint32_t override;
		auto get_samples_done() const noexcept -> uint32_t override;
		auto read_samples(std::span<uint8_t> out) -> uint32_t override;
	private:
		auto load_from_disk(std::string_view path) -> bool;
	private:
		AudioFormat format_;
		std::unique_ptr<drwav, void(*)(drwav*)> wav_;
		uint32_t samples_processed_{ 0u };

		std::vector<uint8_t> file_bytes_{};
	};
}