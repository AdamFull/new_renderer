#include "AudioFile.h"

#include <edge/util/Logger.h>

namespace e2d::audio {
	auto stereo_pcm16_to_mono_float32(std::span<uint8_t> in_out) -> void {
		auto pcm16_in = std::span<drwav_int16>((drwav_int16*)in_out.data(), in_out.size() / sizeof(drwav_int16));
		auto pcm32_out = std::span<float>((float*)in_out.data(), in_out.size() / sizeof(float));

		for (int32_t i = 0; i < static_cast<int32_t>(pcm32_out.size()); ++i) {
			auto left_channel = static_cast<float>(pcm16_in[i * 2]) / 32768.0f;
			auto right_channel = static_cast<float>(pcm16_in[i * 2 + 1]) / 32768.0f;

			pcm32_out[i] = (left_channel + right_channel) * 0.5f;
		}
	}

	auto AudioFile::get_format() const noexcept -> AudioFormat const& {
		return format_;
	}

	auto AudioFile::get_total_samples() const noexcept -> uint32_t {
		return total_samples_;
	}

	auto AudioFile::get_samples_done() const noexcept -> uint32_t {
		return samples_processed_;
	}

	auto AudioFile::read_samples(std::span<uint8_t> out, const AudioFormat& out_format) -> uint32_t {
		auto frames_to_read = out_format.get_number_of_frames_fit(out.size());
		auto frames_readed = _read_samples(out, frames_to_read);

		if (format_.channels == 2 && out_format.channels == 1 && out_format.bit_per_sample == 32) {
			stereo_pcm16_to_mono_float32(out);
		}

		samples_processed_ += frames_readed;
		return out_format.get_number_of_bytes(frames_readed);
	}



	WavFile::WavFile() : wav_(nullptr,
		[](drwav* p) {
			if (p) {
				drwav_uninit(p);
				delete p;
			}
		})
	{}

	auto WavFile::construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile> {
		auto self = std::make_unique<WavFile>();
		self->load_from_memory(data);
		return self;
	}

	auto WavFile::_read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t {
		return drwav_read_pcm_frames_s16(wav_.get(), frames_to_read, reinterpret_cast<drwav_int16*>(out.data()));
	}

	auto WavFile::load_from_memory(std::span<const uint8_t> data) -> bool {
		file_bytes_ = data;

		auto* wav = new drwav;
		if (!drwav_init_memory(wav, file_bytes_.data(), file_bytes_.size(), NULL)) {
			delete wav;
			EDGE_LOGE("[FileSource] Failed to parse wav file");
			return false;
		}

		wav_.reset(wav);

		format_.sample_rate = wav_->sampleRate;
		format_.channels = wav_->channels;
		format_.bit_per_sample = wav_->bitsPerSample;

		total_samples_ = wav_->totalPCMFrameCount * wav_->channels;
	}



	Mp3File::Mp3File() : mp3_(nullptr,
		[](drmp3* p) {
			if (p) {
				drmp3_uninit(p);
				delete p;
			}
		})
	{}

	auto Mp3File::construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile> {
		auto self = std::make_unique<Mp3File>();
		self->load_from_memory(data);
		return self;
	}

	auto Mp3File::_read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t {
		return drmp3_read_pcm_frames_s16(mp3_.get(), frames_to_read, reinterpret_cast<drmp3_int16*>(out.data()));
	}

	auto Mp3File::load_from_memory(std::span<const uint8_t> data) -> bool {
		file_bytes_ = data;

		auto* mp3 = new drmp3;
		if (!drmp3_init_memory(mp3, file_bytes_.data(), file_bytes_.size(), NULL)) {
			delete mp3;
			EDGE_LOGE("[FileSource] Failed to parse mp3 file");
			return false;
		}

		mp3_.reset(mp3);

		format_.sample_rate = mp3_->sampleRate;
		format_.channels = mp3_->channels;
		format_.bit_per_sample = 16;

		total_samples_ = mp3_->totalPCMFrameCount * mp3_->channels;
	}



	FlacFile::FlacFile() : flac_(nullptr,
		[](drflac* p) {
			if (p) {
				drflac_close(p);
			}
		})
	{}

	auto FlacFile::construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile> {
		auto self = std::make_unique<FlacFile>();
		self->load_from_memory(data);
		return self;
	}

	auto FlacFile::_read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t {
		return drflac_read_pcm_frames_s16(flac_.get(), frames_to_read, reinterpret_cast<drflac_int16*>(out.data()));
	}

	auto FlacFile::load_from_memory(std::span<const uint8_t> data) -> bool {
		file_bytes_ = data;

		auto* flac = drflac_open_memory(file_bytes_.data(), file_bytes_.size(), NULL);
		if (!flac) {
			EDGE_LOGE("[FileSource] Failed to parse wav file");
			return false;
		}

		flac_.reset(flac);

		format_.sample_rate = flac_->sampleRate;
		format_.channels = flac_->channels;
		format_.bit_per_sample = flac_->bitsPerSample;

		total_samples_ = flac_->totalPCMFrameCount * flac_->channels;
	}
}