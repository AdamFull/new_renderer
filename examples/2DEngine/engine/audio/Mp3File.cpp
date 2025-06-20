#include "Mp3File.h"
#include "Util.h"

#include <edge/util/Logger.h>

namespace e2d::audio {
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

	auto Mp3File::read_samples(std::span<uint8_t> out) -> uint32_t {
		auto frames_to_read = format_.get_number_of_frames_fit(out.size());
		auto frames_readed = drmp3_read_pcm_frames_s16(mp3_.get(), frames_to_read, reinterpret_cast<drmp3_int16*>(out.data()));
		samples_processed_ += frames_readed;
		return format_.get_number_of_bytes(frames_readed);
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
}