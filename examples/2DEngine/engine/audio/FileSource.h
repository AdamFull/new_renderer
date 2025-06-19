#pragma once

#include <string_view>

#include "AudioSource.h"

namespace e2d::audio {
	class FileSource final : public AudioSource {
	public:
		~FileSource() = default;

		[[nodiscard]] static auto construct(std::string_view path) -> std::unique_ptr<IAudioSource>;

		auto on_data(std::span<float> output, AudioFormat& format) -> void override;
	private:
		auto load_file(std::string_view path) -> bool;
	};
}