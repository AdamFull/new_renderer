#pragma once

#include <glm/glm.hpp>

#include <span>
#include "Common.h"

namespace e2d::audio {
	class IAudioSource {
	public:
		virtual ~IAudioSource() = default;

		virtual auto play() -> void = 0;
		virtual auto stop() -> void = 0;
		virtual auto pause() -> void = 0;

		virtual auto set_playback_position(float sec) -> void = 0;
		virtual auto get_playback_position() const noexcept -> float = 0;
		virtual auto get_duration() const noexcept -> float = 0;

		virtual auto get_format() const noexcept -> const AudioFormat & = 0;

		virtual auto set_world_position(const glm::vec3& pos, bool teleport = false) -> void = 0;
		virtual auto get_world_position() const noexcept -> const glm::vec3& = 0;

		virtual auto update() -> void = 0;
	protected:
		virtual auto on_data(std::span<uint8_t> output, AudioFormat& format) -> size_t = 0;
	};
}