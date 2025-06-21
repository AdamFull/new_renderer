#pragma once

#include "AudioSource.h"
#include "ISynthesisNode.h"

namespace e2d::audio {
	class SynthSource final : public AudioSource {
	public:
		~SynthSource() = default;

		[[nodiscard]] static auto construct(float freq, uint32_t sample_rate) -> std::unique_ptr<IAudioSource>;

		auto on_data(std::span<uint8_t> output, const AudioFormat& format) -> size_t override;
	private:
		std::unique_ptr<ISynthesisNode> synth_;
	};
}