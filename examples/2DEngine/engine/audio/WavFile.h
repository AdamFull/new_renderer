#pragma once

#include <memory>

#include <dr_wav.h>

#include "AudioFile.h"

namespace e2d::audio {
	class WavFile final : public AudioFile {
	public:
		WavFile();
		~WavFile() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile>;

		auto read_samples(std::span<uint8_t> out) -> uint32_t override;
	private:
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drwav, void(*)(drwav*)> wav_;
	};
}