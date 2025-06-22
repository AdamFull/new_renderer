#pragma once

#include <array>
#include <memory>
#include <string_view>

#include "Common.h"
#include "IAudioSource.h"
#include "IAudioFile.h"
#include "ISynthesisNode.h"

namespace e2d::audio {
	constexpr const size_t k_streaming_buffer_count{ 4 };
	constexpr const size_t k_streaming_buffer_size{ 4096 };

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

		auto update() -> void override;
	protected:
		auto initialize_resources() -> void;

		uint32_t total_samples_{ 0u };
		uint32_t samples_processed_{ 0u };
		std::array<uint8_t, k_streaming_buffer_size> sample_buffer_{ 0 };

		uint32_t source_{ ~0u };
		std::array<uint32_t, k_streaming_buffer_count> buffers_{ ~0u };

		AudioFormat format_;
		glm::vec3 last_position_{};
	};

	class SynthSource final : public AudioSource {
	public:
		~SynthSource() = default;

		[[nodiscard]] static auto construct(float freq, uint32_t sample_rate) -> std::unique_ptr<IAudioSource>;

		auto on_data(std::span<uint8_t> output, const AudioFormat& format) -> size_t override;
	private:
		std::unique_ptr<ISynthesisNode> synth_;
	};
}