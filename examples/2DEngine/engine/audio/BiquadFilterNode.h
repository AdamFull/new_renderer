#pragma once

#include "ISynthesisNode.h"

namespace e2d::audio {
	enum class BiquadFilterMode {
		eLowPass, 
		eBandPass, 
		eHighPass,
		eAllPass,
		eNotch
	};

	class BiquadFilterNode final : public ISynthesisNode {
	public:
		BiquadFilterNode();
		~BiquadFilterNode() override = default;

		[[nodiscard]] static auto construct(BiquadFilterMode type = BiquadFilterMode::eLowPass, float cutoff = 1000.0f, float q = 0.707f, uint32_t sample_rate = 44100u) -> std::unique_ptr<ISynthesisNode>;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t override;
		auto attach(std::unique_ptr<ISynthesisNode>&& source) -> void override;

		auto setup(BiquadFilterMode type, float cutoff, float q, uint32_t sample_rate) -> void;
	private:
		std::unique_ptr<ISynthesisNode> source_;

		float b0_, b1_, b2_;  // feedforward coefficients
		float a1_, a2_;      // feedback coefficients (a0 is normalized to 1)

		float x1_, x2_;      // input history
		float y1_, y2_;      // output history

		float cutoff_freq_;
		float q_factor_;

		uint32_t last_sample_rate_;
	};
}