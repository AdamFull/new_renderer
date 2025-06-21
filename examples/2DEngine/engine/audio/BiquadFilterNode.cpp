#include "BiquadFilterNode.h"

#include "Util.h"

namespace e2d::audio {
	BiquadFilterNode::BiquadFilterNode() 
		: b0_(1.0f), b1_(0.0f), b2_(0.0f)
		, a1_(0.0f), a2_(0.0f)
		, x1_(0.0f), x2_(0.0f)
		, y1_(0.0f), y2_(0.0f)
		, cutoff_freq_(1000.0f)
		, q_factor_(0.707f)  // Butterworth response
		, last_sample_rate_(0) {}

	auto BiquadFilterNode::construct(BiquadFilterMode type, float cutoff, float q, uint32_t sample_rate) -> std::unique_ptr<ISynthesisNode> {
		auto self = std::make_unique<BiquadFilterNode>();
		self->setup(type, cutoff, q, sample_rate);
		return self;
	}

	auto BiquadFilterNode::process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t {
		size_t processed_bytes{ 0ull };
		if (source_) {
			processed_bytes = source_->process(out, out_format);
		}

		auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
		for (int32_t i = 0; i < float_out.size(); ++i) {
			const auto in = float_out[i];

			// Direct Form II Transposed implementation
			const auto output = b0_ * in + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;

			// Update delay line
			x2_ = x1_;
			x1_ = in;
			y2_ = y1_;
			y1_ = output;

			float_out[i] = output;
		}

		return processed_bytes;
	}

	auto BiquadFilterNode::attach(std::unique_ptr<ISynthesisNode>&& source) -> void {
		source_ = std::move(source);
	}

	auto BiquadFilterNode::setup(BiquadFilterMode type, float cutoff, float q, uint32_t sample_rate) -> void {
		cutoff_freq_ = cutoff;
		last_sample_rate_ = sample_rate;
		q_factor_ = q;

		const float omega = math::two_pi * cutoff_freq_ / static_cast<float>(sample_rate);
		const float cos_omega = std::cos(omega);
		const float sin_omega = std::sin(omega);
		const float alpha = sin_omega / (2.0f * q_factor_);

		float a0, a1, a2, b0, b1, b2;

		switch (type) {
		case BiquadFilterMode::eLowPass:
			b1 = 1.0f - cos_omega;
			b0 = b2 = b1 * 0.5f;
			a0 = 1.0f + alpha;
			a1 = -2.0f * cos_omega;
			a2 = 1.0f - alpha;
			break;

		case BiquadFilterMode::eHighPass:
			b0 = b2 = (1.0f + cos_omega) * 0.5f;
			b1 = -(1.0f + cos_omega);
			a0 = 1.0f + alpha;
			a1 = -2.0f * cos_omega;
			a2 = 1.0f - alpha;
			break;

		case BiquadFilterMode::eBandPass:
			b0 = sin_omega * 0.5f;
			b1 = 0.0f;
			b2 = -sin_omega * 0.5f;
			a0 = 1.0f + alpha;
			a1 = -2.0f * cos_omega;
			a2 = 1.0f - alpha;
			break;

		case BiquadFilterMode::eNotch:
			b0 = 1.0f;
			b1 = -2.0f * cos_omega;
			b2 = 1.0f;
			a0 = 1.0f + alpha;
			a1 = -2.0f * cos_omega;
			a2 = 1.0f - alpha;
			break;

		case BiquadFilterMode::eAllPass:
			b0 = 1.0f - alpha;
			b1 = -2.0f * cos_omega;
			b2 = 1.0f + alpha;
			a0 = 1.0f + alpha;
			a1 = -2.0f * cos_omega;
			a2 = 1.0f - alpha;
			break;
		}

		// Normalize coefficients by a0
		b0_ = b0 / a0;
		b1_ = b1 / a0;
		b2_ = b2 / a0;
		a1_ = a1 / a0;
		a2_ = a2 / a0;
	}
}