#pragma once

#include <array>
#include <algorithm>

#include "ISynthesisNode.h"

#include "DSP.h"

namespace e2d::audio {
	class BiquadFilterNode final : public ISynthesisNode {
	public:
		~BiquadFilterNode() override = default;

		[[nodiscard]] static auto construct(BiquadFilter::Mode type = BiquadFilter::Mode::eLowPass, float cutoff = 1000.0f, float q = 0.707f, float gain_db = 0.0f, uint32_t sample_rate = 44100u) -> std::unique_ptr<ISynthesisNode>;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t override;
		auto attach(std::unique_ptr<ISynthesisNode>&& source) -> void override;
	private:
		std::unique_ptr<ISynthesisNode> source_;
		BiquadFilter filter_;
	};

	class PhaserFilterNode final : public ISynthesisNode {
	public:
		~PhaserFilterNode() override = default;

		[[nodiscard]] static auto construct(float rate = 0.5f, float depth = 0.75f, float feedback = 0.3f, float mix = 0.5f, uint32_t sample_rate = 44100u) -> std::unique_ptr<ISynthesisNode>;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t override;
		auto attach(std::unique_ptr<ISynthesisNode>&& source) -> void override;
	private:
		std::unique_ptr<ISynthesisNode> source_;
		PhaserFilter<16> filter_;
	};

	class Equalizer5BandNode final : public ISynthesisNode {
	public:
		~Equalizer5BandNode() override = default;

		[[nodiscard]] static auto construct(uint32_t sample_rate = 44100u) -> std::unique_ptr<ISynthesisNode>;

		auto process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t override;
		auto attach(std::unique_ptr<ISynthesisNode>&& source) -> void override;
	private:
		std::unique_ptr<ISynthesisNode> source_;
		EqualizerFilter5Band filter_;
	};
}