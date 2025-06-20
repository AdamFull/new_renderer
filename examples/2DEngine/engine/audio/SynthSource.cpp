#include "SynthSource.h"
#include "OscillatorNode.h"

#include "BackendHelper.h"
#include "Util.h"

#include <AL/al.h>

namespace e2d::audio {
	auto SynthSource::construct(float freq, uint32_t sample_rate) -> std::unique_ptr<IAudioSource> {
		auto self = std::make_unique<SynthSource>();
		self->initialize_resources();
		self->format_.channels = 1;
		self->format_.bit_per_sample = 32;
		self->format_.sample_rate = 44100;
		self->synth_ = OscillatorNode::construct(WaveType::eSaw, 440.0f, 0.0f);
		self->prefetch();
		return self;
	}

	auto SynthSource::on_data(std::span<uint8_t> output, AudioFormat& format) -> size_t {
		return synth_->process(output, format);
	}

	auto SynthSource::prefetch() -> void {
		auto al_format = get_internal_format(format_);

		// Prefetch data
		std::array<uint8_t, k_streaming_buffer_size> streaming_buffer_{ 0 };
		for (int32_t i = 0; i < k_streaming_buffer_count; ++i) {
			auto bytes_readed = synth_->process(streaming_buffer_, format_);
			alCall(alBufferData, buffers_[i], al_format, streaming_buffer_.data(), bytes_readed, format_.sample_rate);
		}

		alCall(alSourceQueueBuffers, source_, k_streaming_buffer_count, &buffers_[0]);
	}
}