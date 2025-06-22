#include "BiquadFilterNode.h"

#include "Util.h"

namespace e2d::audio {
	auto BiquadFilterNode::construct(BiquadFilter::Mode type, float cutoff, float q, float gain_db, uint32_t sample_rate) -> std::unique_ptr<ISynthesisNode> {
		auto self = std::make_unique<BiquadFilterNode>();
		self->filter_ = BiquadFilter(cutoff, gain_db, q, sample_rate, type);
		return self;
	}

	auto BiquadFilterNode::process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t {
		size_t processed_bytes{ 0ull };
		if (source_) {
			processed_bytes = source_->process(out, out_format);
		}

		auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
		for (int32_t i = 0; i < float_out.size(); ++i) {
			float_out[i] = filter_.process(float_out[i]);
		}

		return processed_bytes;
	}

	auto BiquadFilterNode::attach(std::unique_ptr<ISynthesisNode>&& source) -> void {
		source_ = std::move(source);
	}



	auto PhaserFilterNode::construct(float rate, float depth, float feedback, float mix, uint32_t sample_rate) -> std::unique_ptr<ISynthesisNode> {
		auto self = std::make_unique<PhaserFilterNode>();
		self->filter_ = PhaserFilter<16>{ rate, depth, feedback, mix, sample_rate };
		return self;
	}

	auto PhaserFilterNode::process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t {
		size_t processed_bytes{ 0ull };
		if (source_) {
			processed_bytes = source_->process(out, out_format);
		}

		auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
		for (int32_t i = 0; i < float_out.size(); ++i) {
			float_out[i] = filter_.process(float_out[i]);
		}

		return processed_bytes;
	}

	auto PhaserFilterNode::attach(std::unique_ptr<ISynthesisNode>&& source) -> void {
		source_ = std::move(source);
	}


	auto Equalizer5BandNode::construct(uint32_t sample_rate) -> std::unique_ptr<ISynthesisNode> {
		auto self = std::make_unique<Equalizer5BandNode>();
		self->filter_ = EqualizerFilter5Band{ sample_rate };
		return self;
	}

	auto Equalizer5BandNode::process(std::span<uint8_t> out, const AudioFormat& out_format) -> size_t {
		size_t processed_bytes{ 0ull };
		if (source_) {
			processed_bytes = source_->process(out, out_format);
		}

		auto float_out = std::span<float>((float*)out.data(), out.size() / sizeof(float));
		for (int32_t i = 0; i < float_out.size(); ++i) {
			float_out[i] = filter_.process(float_out[i]);
		}

		return processed_bytes;
	}

	auto Equalizer5BandNode::attach(std::unique_ptr<ISynthesisNode>&& source) -> void {
		source_ = std::move(source);
	}
}