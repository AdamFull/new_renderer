#include "OscillatorNode.h"

#include <array>

#include "Util.h"

namespace e2d::audio {
	auto OscillatorNode::construct(WaveType wave_form, float freq, float start_phase) -> std::unique_ptr<ISynthesisNode> {
		auto self = std::make_unique<OscillatorNode>();
		self->type_ = wave_form;
		self->frequency_ = freq;
		self->start_phase_ = start_phase;
		self->phase_ = start_phase;
		return self;
	}

	auto OscillatorNode::process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t {
		auto delta_phase = math::two_pi * frequency_ / static_cast<float>(out_format.sample_rate);

		auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
		for (int32_t i = 0; i < float_out.size(); ++i) {
			switch (type_)
			{
			case e2d::audio::WaveType::eSine:
				float_out[i] = std::sinf(phase_);
				break;
			case e2d::audio::WaveType::eSquare:
				float_out[i] = (phase_ < math::pi) ? 1.0f : -1.0f;
				break;
			case e2d::audio::WaveType::eSaw:
				float_out[i] = (2.0f * phase_ / math::two_pi) - 1.0f;
				break;
			case e2d::audio::WaveType::eTriangle: {
				const float normalized = phase_ / math::two_pi;
				float_out[i] = (normalized < 0.5f) ? (4.0f * normalized - 1.0f) : (3.0f - 4.0f * normalized);
				break;
			}
			default:
				break;
			}

			phase_ += delta_phase;
			if (phase_ >= math::two_pi) {
				phase_ -= math::two_pi;
			}
		}

		return out.size();
	}
}