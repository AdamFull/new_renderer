#pragma once

#include <memory>

#include "ISynthesisNode.h"

namespace e2d::audio {
	class LowPassFilterNode final : public ISynthesisNode {
	public:
		~LowPassFilterNode() override = default;

		[[nodiscard]] static auto construct(float cutoff = 1000.0f) -> std::unique_ptr<ISynthesisNode>;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t override;
	private:
		float cutoff_freq_{ 1000.0f };
		float last_out_{ 0.0f };
	};
}