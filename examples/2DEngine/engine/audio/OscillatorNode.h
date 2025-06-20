#pragma once

#include <memory>

#include "ISynthesisNode.h"

namespace e2d::audio {


	class OscillatorNode final : public ISynthesisNode {
	public:
		~OscillatorNode() override = default;

		[[nodiscard]] static auto construct(WaveType wave_form, float freq, float start_phase) -> std::unique_ptr<ISynthesisNode>;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t override;
	private:
		WaveType type_{};
		float phase_{ 0.0f };
		float start_phase_{ 0.0f };
		float frequency_{ 0.0f };
	};
}