#pragma once

#include <memory>

#include "Common.h"
#include "IAudioSource.h"

namespace e2d::audio {
	class AudioSource : public IAudioSource {
	public:
		virtual ~AudioSource() override;

		auto play() -> void final override;
		auto stop() -> void final override;
		auto pause() -> void final override;

		auto set_playback_position(float sec) -> void final override;
		auto get_playback_position() const noexcept -> float final override;
		auto get_duration() const noexcept -> float final override;

		auto get_format() const noexcept -> const AudioFormat& final override;

		auto set_world_position(const glm::vec3& pos, bool teleport = false) -> void final override;
		auto get_world_position() const noexcept -> const glm::vec3& final override;
	protected:
		auto initialize_resources() -> void;

		uint32_t source_{ ~0u };
		uint32_t buffer_{ ~0u };

		AudioFormat format_;
		glm::vec3 last_position_{};
	};
}