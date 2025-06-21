#include "AudioSource.h"
#include "Util.h"

#include <AL/al.h>

#include "AudioFile.h"
#include "BackendHelper.h"

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/fs/fs_helper.h>
#include <edge/util/Logger.h>

namespace e2d::audio {
	AudioSource::~AudioSource() {
		alCall(alDeleteSources, 1, &source_);
		alCall(alDeleteBuffers, k_streaming_buffer_count, &buffers_[0]);
	}

	auto AudioSource::play() -> void {
		// Prefetch audio data for begin playong
		auto al_format = get_internal_format(format_);

		for (int32_t i = 0; i < k_streaming_buffer_count; ++i) {
			auto bytes_readed = on_data(sample_buffer_, format_);
			alCall(alBufferData, buffers_[i], al_format, sample_buffer_.data(), bytes_readed, format_.sample_rate);
		}

		alCall(alSourceQueueBuffers, source_, k_streaming_buffer_count, &buffers_[0]);

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



	namespace ext {
		using namespace edge;
		constexpr auto wav = ".wav"_hash32;
		constexpr auto wave = ".wave"_hash32;
		constexpr auto mp3 = ".mp3"_hash32;
		constexpr auto ogg = ".ogg"_hash32;
		constexpr auto flac = ".flac"_hash32;
		constexpr auto opus = ".opus"_hash32;
	}

	auto FileSource::construct(std::string_view path) -> std::unique_ptr<IAudioSource> {
		auto self = std::make_unique<FileSource>();
		self->initialize_resources();
		self->load_file(path);
		return self;
	}

	auto FileSource::on_data(std::span<uint8_t> output, const AudioFormat& format) -> size_t {
		return file_->read_samples(output, format);
	}

	auto FileSource::load_file(std::string_view path) -> bool {
		if (!edge::fs::exists(path)) {
			EDGE_LOGE("[FileSource] File not found");
			return false;
		}

		if (!edge::fs::read_file(path, file_bytes_)) {
			EDGE_LOGE("[FileSource] Failed to load file");
			return false;
		}

		auto file_ext = edge::fs::get_ext(path);
		auto ext_hash = edge::murmur_hash(file_ext);
		switch (ext_hash)
		{
		case ext::wav:
		case ext::wave: {
			file_ = WavFile::construct(file_bytes_);
			break;
		};
		case ext::mp3: {
			file_ = Mp3File::construct(file_bytes_);
			break;
		};
		case ext::ogg: {
			//file_ = OggFile::construct(file_bytes_);
			break;
		};
		case ext::flac: {
			file_ = FlacFile::construct(file_bytes_);
			break;
		};
		case ext::opus: {
			//file_ = OpusFile::construct(file_bytes_);
			break;
		};
		default:
			break;
		}

		format_ = file_->get_format();

		return file_.get();
	}
}