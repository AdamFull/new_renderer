#include "SynthSource.h"
#include "OscillatorNode.h"
#include "BiquadFilterNode.h"
#include "FileInputNode.h"

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
		//self->synth_ = LowPassFilterNode::construct(150.0f);
		self->synth_ = BiquadFilterNode::construct(BiquadFilterMode::eLowPass, 150.0f);
		self->synth_->attach(FileInputNode::construct("/internal_res/assets/iamtheprotectorofthissystem.mp3"));
		return self;
	}

	auto SynthSource::on_data(std::span<uint8_t> output, const AudioFormat& format) -> size_t {
		return synth_->process(output, format);
	}
}