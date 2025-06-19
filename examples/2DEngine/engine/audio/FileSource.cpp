#include "FileSource.h"
#include "BackendHelper.h"

#include <AL/al.h>

#include <dr_wav.h>

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/util/Logger.h>

namespace e2d::audio {
	auto FileSource::construct(std::string_view path) -> std::unique_ptr<IAudioSource> {
		auto self = std::make_unique<FileSource>();
		self->initialize_resources();
		self->load_file(path);
		return self;
	}

	auto FileSource::on_data(std::span<float> output, AudioFormat& format) -> void {

	}

	auto FileSource::load_file(std::string_view path) -> bool {
		if (!edge::fs::exists(path)) {
			EDGE_LOGE("[FileSource] File not found");
			return false;
		}

		std::vector<uint8_t> file_bytes{};
		if (!edge::fs::read_file(path, file_bytes)) {
			EDGE_LOGE("[FileSource] Failed to load file");
			return false;
		}

		drwav wav;
		if (!drwav_init_memory(&wav, file_bytes.data(), file_bytes.size() , NULL)) {
			EDGE_LOGE("[FileSource] Failed to parse wav file");
			return false;
		}

		format_.channels = wav.channels;
		format_.sample_rate = wav.sampleRate;
		format_.bit_per_sample = wav.bitsPerSample;

		auto total_samples = wav.totalPCMFrameCount * wav.channels;
		auto buffer_size = total_samples * sizeof(drwav_int16);

		std::vector<drwav_int16> samples{};
		samples.resize(total_samples);

		drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, samples.data());

		ALenum al_format{ AL_FORMAT_STEREO8 };
		if (format_.channels == 1 && format_.bit_per_sample == 8)
			al_format = AL_FORMAT_MONO8;
		else if (format_.channels == 1 && format_.bit_per_sample == 16)
			al_format = AL_FORMAT_MONO16;
		else if (format_.channels == 2 && format_.bit_per_sample == 8)
			al_format = AL_FORMAT_STEREO8;
		else if (format_.channels == 2 && format_.bit_per_sample == 16)
			al_format = AL_FORMAT_STEREO16;

		alCall(alBufferData, buffer_, al_format, samples.data(), buffer_size, format_.sample_rate);
		alCall(alSourcei, source_, AL_BUFFER, buffer_);

		return true;
	}
}