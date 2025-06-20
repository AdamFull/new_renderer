#include "WavFile.h"
#include "Util.h"

#include <edge/util/Logger.h>

namespace e2d::audio {
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

	auto WavFile::read_samples(std::span<uint8_t> out) -> uint32_t {
		auto frames_to_read = format_.get_number_of_frames_fit(out.size());
		auto frames_readed = drwav_read_pcm_frames_s16(wav_.get(), frames_to_read, reinterpret_cast<drwav_int16*>(out.data()));
		samples_processed_ += frames_readed;
		return format_.get_number_of_bytes(frames_readed);
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
}