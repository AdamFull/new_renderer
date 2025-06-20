#include "LowPassFilterNode.h"

#include "Util.h"

namespace e2d::audio {
	auto LowPassFilterNode::construct(float cutoff) -> std::unique_ptr<ISynthesisNode> {
		auto self = std::make_unique<LowPassFilterNode>();
		self->cutoff_freq_ = cutoff;
		return self;
	}

	auto LowPassFilterNode::process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t {
		// TODO: Call child node before filtering...

		float rc = 1.0f / (math::two_pi * cutoff_freq_);
		float dt = 1.0f / static_cast<float>(out_format.sample_rate);
		float alpha = dt / (rc + dt);

		auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
		for (int32_t i = 0; i < float_out.size(); ++i) {
			last_out_ = alpha * float_out[i] + (1.0f - alpha) * last_out_;
			float_out[i] = last_out_;
		}

		// TODO: Return real number of bytes
		return out.size();
	}
}