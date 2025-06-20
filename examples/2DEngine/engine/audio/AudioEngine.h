#pragma once

#include <memory>
#include <thread>
#include <mutex>

#include "IAudioEngine.h"
#include "IAudioSource.h"

#include <edge/util/utl_concurrent.hpp>

namespace e2d::audio {
	class AudioEngine final : public IAudioEngine {
	public:
		~AudioEngine() override;

		[[nodiscard]] static auto construct() -> std::unique_ptr<IAudioEngine>;

		auto add_source(std::unique_ptr<IAudioSource>&& source) -> AudioSourceHandle override;

		auto play_source(AudioSourceHandle handle) -> void override;
		auto pause_source(AudioSourceHandle handle) -> void override;
		auto stop_source(AudioSourceHandle handle) -> void override;
	private:
		auto initialize_context() -> bool;
		auto run_streaming_thread() -> void;
		auto audio_thread(std::stop_token token) -> void;
	private:
		struct _internal;
		std::unique_ptr<_internal> internal_;

		std::jthread audio_thread_;
		std::mutex source_map_mutex_;
		std::vector<std::unique_ptr<IAudioSource>> source_map_;

		edge::utl::spsc_ring_buffer<AudioEngineCommand, 32> command_list_;
	};
}