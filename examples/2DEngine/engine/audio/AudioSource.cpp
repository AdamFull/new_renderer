#include "AudioSource.h"
#include "Util.h"

#include "BackendHelper.h"

#include <AL/al.h>

namespace e2d::audio {
	AudioSource::~AudioSource() {
		alCall(alDeleteSources, 1, &source_);
		alCall(alDeleteBuffers, k_streaming_buffer_count, &buffers_[0]);
	}

	auto AudioSource::play() -> void {
		alCall(alSourcePlay, source_);
	}

	auto AudioSource::stop() -> void {
		alCall(alSourceStop, source_);
	}

	auto AudioSource::pause() -> void {
		alCall(alSourcePause, source_);
	}

	auto AudioSource::set_playback_position(float sec) -> void {
		// TODO:
	}

	auto AudioSource::get_playback_position() const noexcept -> float {
		return 0.0f;
	}

	auto AudioSource::get_duration() const noexcept -> float {
		return 0.0f;
	}

	auto AudioSource::get_format() const noexcept -> const AudioFormat& {
		return format_;
	}

	auto AudioSource::set_world_position(const glm::vec3& pos, bool teleport) -> void {
		auto velocity = teleport ? glm::vec3(0.0f) : (last_position_ - pos);

		alCall(alSource3f, source_, AL_POSITION, pos.x, pos.y, pos.z);
		alCall(alSource3f, source_, AL_VELOCITY, velocity.x, velocity.y, velocity.z);

		last_position_ = pos;
	}

	auto AudioSource::get_world_position() const noexcept -> const glm::vec3& {
		return last_position_;
	}

	auto AudioSource::update() -> void {
		ALint state = AL_PLAYING;
		alCall(alGetSourcei, source_, AL_SOURCE_STATE, &state);

		if (state != AL_PLAYING) {
			return;
		}

		ALint buffers_processed{ 0 };
		alCall(alGetSourcei, source_, AL_BUFFERS_PROCESSED, &buffers_processed);

		if (buffers_processed <= 0) {
			return;
		}

		while (buffers_processed--) {
			ALuint buffer;
			alCall(alSourceUnqueueBuffers, source_, 1, &buffer);

			auto samples_readed = on_data(sample_buffer_, format_);

			auto al_format = get_internal_format(format_);
			alCall(alBufferData, buffer, al_format, sample_buffer_.data(), samples_readed, format_.sample_rate);
			alCall(alSourceQueueBuffers, source_, 1, &buffer);
		}
	}

	auto AudioSource::initialize_resources() -> void {
		alCall(alGenBuffers, k_streaming_buffer_count, &buffers_[0]);

		alCall(alGenSources, 1, &source_);
		alCall(alSourcef, source_, AL_PITCH, 1.0f);
		alCall(alSourcef, source_, AL_GAIN, 1.0f);
		alCall(alSource3f, source_, AL_POSITION, 0.0f, 0.0f, 0.0f);
		alCall(alSource3f, source_, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		alCall(alSourcei, source_, AL_LOOPING, AL_FALSE);
	}
}