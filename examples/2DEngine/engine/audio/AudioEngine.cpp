#include "AudioEngine.h"

#include "BackendHelper.h"

#include <chrono>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <edge/util/Logger.h>

namespace e2d::audio {
	struct AudioEngine::_internal {
		ALCdevice* dev_{ nullptr };
		ALCcontext* ctx_{ nullptr };
	};

	AudioEngine::~AudioEngine() {
		ALCboolean context_made_current{ false };
		if (!alcCall(alcMakeContextCurrent, context_made_current, internal_->dev_, internal_->ctx_) || context_made_current != ALC_TRUE) {
			EDGE_LOGE("[AudioEngine] Failed to reset current context");
		}

		if (!alcCall(alcDestroyContext, internal_->dev_, internal_->ctx_)) {
			EDGE_LOGE("[AudioEngine] Failed to destroy OpenAL context");
		}

		ALCboolean device_closed;
		if (!alcCall(alcCloseDevice, device_closed, internal_->dev_, internal_->dev_)) {
			EDGE_LOGE("[AudioEngine] Failed to close OpenAL device");
		}
	}

	auto AudioEngine::construct() -> std::unique_ptr<IAudioEngine> {
		auto self = std::make_unique<AudioEngine>();
		self->initialize_context();
		self->run_streaming_thread();
		return self;
	}

	auto AudioEngine::add_source(std::unique_ptr<IAudioSource>&& source) -> AudioSourceHandle {
		std::lock_guard lock{ source_map_mutex_ };
		auto id = static_cast<AudioSourceHandle>(source_map_.size());
		source_map_.emplace_back(std::move(source));
		return id;
	}

	auto AudioEngine::play_source(AudioSourceHandle handle) -> void {
		AudioEngineCommand cmd;
		cmd.type = CommandType::ePlay;
		cmd.handle = handle;

		command_list_.try_push(cmd);
	}

	auto AudioEngine::pause_source(AudioSourceHandle handle) -> void {
		AudioEngineCommand cmd;
		cmd.type = CommandType::ePause;
		cmd.handle = handle;

		command_list_.try_push(cmd);
	}

	auto AudioEngine::stop_source(AudioSourceHandle handle) -> void {
		AudioEngineCommand cmd;
		cmd.type = CommandType::eStop;
		cmd.handle = handle;

		command_list_.try_push(cmd);
	}

	auto AudioEngine::initialize_context() -> bool {
		internal_ = std::make_unique<_internal>();

		internal_->dev_ = alcOpenDevice(nullptr);
		if (!internal_->dev_) {
			EDGE_LOGE("[AudioEngine] Failed to open OpenAL device");
			return false;
		}

		if (!alcCall(alcCreateContext, internal_->ctx_, internal_->dev_, internal_->dev_, nullptr) || !internal_->ctx_)
		{
			EDGE_LOGE("[AudioEngine] Failed to create OpenAL context");
			alcCloseDevice(internal_->dev_);
			return false;
		}

		ALCboolean context_made_current{ false };
		if (!alcCall(alcMakeContextCurrent, context_made_current, internal_->dev_, internal_->ctx_) || context_made_current != ALC_TRUE)
		{
			EDGE_LOGE("[AudioEngine] Failed to make OpenAL context current");
			alcDestroyContext(internal_->ctx_);
			alcCloseDevice(internal_->dev_);
			return false;
		}

		// Iterate audio devices for debug
		auto devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
		auto device = devices;
		auto next = devices + 1;

		while (device && *device != '\0' && next && *next != '\0') {
			EDGE_LOGD("[AudioEngine] Audio Device: {}", device);
			auto len = std::strlen(device);
			device += len + 1;
			next += len + 2;
		}

		auto device_name = alcGetString(internal_->dev_, ALC_DEVICE_SPECIFIER);
		EDGE_LOGD("[AudioEngine] Selected Audio Device: {}", device_name);
		EDGE_LOGD("[AudioEngine] Audio engine initialized successfully");

		return true;
	}

	auto AudioEngine::run_streaming_thread() -> void {
		audio_thread_ = std::jthread(std::bind_front(&AudioEngine::audio_thread, this));
	}

	auto AudioEngine::audio_thread(std::stop_token token) -> void {
		while (!token.stop_requested()) {
			std::lock_guard lock{ source_map_mutex_ };

			while (!command_list_.empty()) {
				AudioEngineCommand cmd;
				if (command_list_.try_pop(cmd)) {
					switch (cmd.type)
					{
					case CommandType::ePlay: {
						source_map_[cmd.handle]->play();
						break;
					}
					case CommandType::ePause: {
						source_map_[cmd.handle]->pause();
						break;
					}
					case CommandType::eStop: {
						source_map_[cmd.handle]->stop();
						break;
					}
					default:
						break;
					}
				}
			}

			for (auto& source : source_map_) {
				source->update();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		}
	}
}