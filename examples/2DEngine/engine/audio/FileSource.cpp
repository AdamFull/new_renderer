#include "FileSource.h"
#include "BackendHelper.h"
#include "Util.h"

#include <AL/al.h>

#include "FlacFile.h"
#include "Mp3File.h"
#include "WavFile.h"

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/fs/fs_helper.h>
#include <edge/util/Logger.h>

namespace e2d::audio {
	namespace ext {
		using namespace edge;
		constexpr auto wav = ".wav"_hash32;
		constexpr auto wave = ".wave"_hash32;
		constexpr auto mp3 = ".mp3"_hash32;
		constexpr auto ogg = ".ogg"_hash32;
		constexpr auto flac = ".flac"_hash32;
		constexpr auto opus = ".opus"_hash32;
	}

	auto FileSource::construct(std::string_view path) -> std::unique_ptr<IAudioSource> {
		auto self = std::make_unique<FileSource>();
		self->initialize_resources();
		self->load_file(path);
		return self;
	}

	auto FileSource::on_data(std::span<uint8_t> output, AudioFormat& format) -> size_t {
		return file_->read_samples(output);
	}

	auto FileSource::load_file(std::string_view path) -> bool {
		if (!edge::fs::exists(path)) {
			EDGE_LOGE("[FileSource] File not found");
			return false;
		}

		if (!edge::fs::read_file(path, file_bytes_)) {
			EDGE_LOGE("[FileSource] Failed to load file");
			return false;
		}

		auto file_ext = edge::fs::get_ext(path);
		auto ext_hash = edge::murmur_hash(file_ext);
		switch (ext_hash)
		{
		case ext::wav:
		case ext::wave: {
			file_ = WavFile::construct(file_bytes_);
			break;
		};
		case ext::mp3: {
			file_ = Mp3File::construct(file_bytes_);
			break;
		};
		case ext::ogg: {
			//file_ = OggFile::construct(file_bytes_);
			break;
		};
		case ext::flac: {
			file_ = FlacFile::construct(file_bytes_);
			break;
		};
		case ext::opus: {
			//file_ = OpusFile::construct(file_bytes_);
			break;
		};
		default:
			break;
		}

		format_ = file_->get_format();

		auto al_format = get_internal_format(format_);

		// Prefetch data
		std::array<uint8_t, K_streaming_buffer_size> streaming_buffer_;
		for (int32_t i = 0; i < k_streaming_buffer_count; ++i) {
			auto bytes_readed = file_->read_samples(streaming_buffer_);
			alCall(alBufferData, buffers_[i], al_format, streaming_buffer_.data(), bytes_readed, format_.sample_rate);
		}

		alCall(alSourceQueueBuffers, source_, k_streaming_buffer_count, &buffers_[0]);

		return file_.get();
	}
}