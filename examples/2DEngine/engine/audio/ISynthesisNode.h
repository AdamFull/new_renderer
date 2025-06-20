#pragma once

#include <span>

#include "Common.h"

namespace e2d::audio {
	class ISynthesisNode {
	public:
		virtual ~ISynthesisNode() = default;

		virtual auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t = 0;
	};
}