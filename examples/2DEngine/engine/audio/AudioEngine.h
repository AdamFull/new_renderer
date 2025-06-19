#pragma once

#include <memory>
#include <thread>

#include "IAudioEngine.h"
#include "IAudioSource.h"

namespace e2d::audio {
	class AudioEngine final : public IAudioEngine {
	public:
		~AudioEngine() override;

		[[nodiscard]] static auto construct() -> std::unique_ptr<IAudioEngine>;
	private:
		auto initialize_context() -> bool;
		auto run_streaming_thread() -> void;
		auto audio_thread(std::stop_token token) -> void;
	private:
		struct _internal;
		std::unique_ptr<_internal> internal_;

		std::jthread audio_thread_;
	};
}