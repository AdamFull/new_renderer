#include "WavFile.h"
#include "Util.h"

#include <edge/fs/fs_virtual_filesystem.h>
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

	auto WavFile::construct(std::string_view path) -> std::unique_ptr<IAudioFile> {
		auto self = std::make_unique<WavFile>();
		self->load_from_disk(path);
		return self;
	}

	auto WavFile::get_format() const noexcept -> AudioFormat const& {
		return format_;
	}

	auto WavFile::get_total_samples() const noexcept -> uint32_t {
		return wav_->totalPCMFrameCount * wav_->channels;
	}

	auto WavFile::get_samples_done() const noexcept -> uint32_t {
		return samples_processed_;
	}

	auto WavFile::read_samples(std::span<uint8_t> out) -> uint32_t {
		auto bytes_per_sample = format_.bit_per_sample / 8;
		auto frames_to_read = out.size() / (bytes_per_sample * format_.channels);

		auto frames_readed = drwav_read_pcm_frames_s16(wav_.get(), frames_to_read, reinterpret_cast<drwav_int16*>(out.data()));
		samples_processed_ += frames_readed;
		return frames_readed * bytes_per_sample * format_.channels;
	}

	auto WavFile::load_from_disk(std::string_view path) -> bool {
		if (!edge::fs::exists(path)) {
			EDGE_LOGE("[FileSource] File not found");
			return false;
		}

		if (!edge::fs::read_file(path, file_bytes_)) {
			EDGE_LOGE("[FileSource] Failed to load file");
			return false;
		}

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
	}
}