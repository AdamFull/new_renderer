#pragma once

#include "IAudioSource.h"

namespace e2d::audio {
	using AudioSourceHandle = uint32_t;

	enum class CommandType {
		ePlay,
		ePause,
		eStop
	};

	struct AudioEngineCommand {
		CommandType type{};

		union {
			AudioSourceHandle handle;
		};
	};

	class IAudioEngine {
	public:
		virtual ~IAudioEngine() = default;

		virtual auto add_source(std::unique_ptr<IAudioSource>&& source) -> AudioSourceHandle = 0;
		virtual auto get_source(AudioSourceHandle handle) -> IAudioSource* = 0;

		virtual auto play_source(AudioSourceHandle handle) -> void = 0;
		virtual auto pause_source(AudioSourceHandle handle) -> void = 0;
		virtual auto stop_source(AudioSourceHandle handle) -> void = 0;
	};
}