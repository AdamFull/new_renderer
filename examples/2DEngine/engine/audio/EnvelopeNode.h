#pragma once

#include "ISynthesisNode.h"

namespace e2d::audio {
	class EnvelopeNode final : public ISynthesisNode {
	public:
		~EnvelopeNode() override = default;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t;
	};
}