#pragma once

#include <string_view>

#include "AudioSource.h"
#include "IAudioFile.h"

namespace e2d::audio {
	class FileSource final : public AudioSource {
	public:
		~FileSource() = default;

		[[nodiscard]] static auto construct(std::string_view path) -> std::unique_ptr<IAudioSource>;

		auto on_data(std::span<uint8_t> output, AudioFormat& format) -> size_t override;
	private:
		auto load_file(std::string_view path) -> bool;
	private:
		std::unique_ptr<IAudioFile> file_;
		std::vector<uint8_t> file_bytes_{};
	};
}