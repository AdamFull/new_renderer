#pragma once

#include <memory>

#include <dr_mp3.h>

#include "AudioFile.h"

namespace e2d::audio {
	class Mp3File final : public AudioFile {
	public:
		Mp3File();
		~Mp3File() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile>;

		auto read_samples(std::span<uint8_t> out) -> uint32_t override;
	private:
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drmp3, void(*)(drmp3*)> mp3_;
	};
}