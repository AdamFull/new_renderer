#include "FlacFile.h"
#include "Util.h"

#include <edge/util/Logger.h>

namespace e2d::audio {
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

	auto FlacFile::read_samples(std::span<uint8_t> out) -> uint32_t {
		auto frames_to_read = format_.get_number_of_frames_fit(out.size());
		auto frames_readed = drflac_read_pcm_frames_s16(flac_.get(), frames_to_read, reinterpret_cast<drflac_int16*>(out.data()));
		samples_processed_ += frames_readed;
		return format_.get_number_of_bytes(frames_readed);
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