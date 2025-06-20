#pragma once

#include <memory>

#include <dr_flac.h>

#include "AudioFile.h"

namespace e2d::audio {
	class FlacFile final : public AudioFile {
	public:
		FlacFile();
		~FlacFile() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile>;

		auto read_samples(std::span<uint8_t> out) -> uint32_t override;
	private:
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drflac, void(*)(drflac*)> flac_;
	};
}