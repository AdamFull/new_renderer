#include "AudioSource.h"
#include "Util.h"

#include "OscillatorNode.h"
#include "BiquadFilterNode.h"
#include "FileInputNode.h"

#include "EffectNodes.h"

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



	auto SynthSource::construct(float freq, uint32_t sample_rate) -> std::unique_ptr<IAudioSource> {
		auto self = std::make_unique<SynthSource>();
		self->initialize_resources();
		self->format_.channels = 1;
		self->format_.bit_per_sample = 32;
		self->format_.sample_rate = 44100;
		//self->synth_ = LowPassFilterNode::construct(150.0f);
		//self->synth_ = BiquadFilterNode::construct(BiquadFilter::Mode::eLowShelf, 150.0f, 0.707f, 30.0f);
		//self->synth_ = PhaserFilterNode::construct(0.8f, 0.9f, 0.6f, 0.7f);
		//auto eq = Equalizer5BandNode::construct();
		//self->synth_ = std::move(eq);
		//self->synth_->attach(FileInputNode::construct("/internal_res/assets/iamtheprotectorofthissystem.mp3"));

		self->graph_ = std::make_unique<EffectGraph>();
		auto output_node_id = self->graph_->add_node(std::make_unique<OutputGraphNode>());
		self->graph_->set_output_node(output_node_id);

		return self;
	}

	auto SynthSource::on_data(std::span<uint8_t> output, const AudioFormat& format) -> size_t {
		return graph_->process(output, format);
		//return synth_->process(output, format);
	}

	auto SynthSource::get_graph() -> EffectGraph& {
		return *graph_;
	}
}