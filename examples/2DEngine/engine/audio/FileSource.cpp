#include "FileSource.h"
#include "BackendHelper.h"
#include "Util.h"

#include <AL/al.h>

#include "WavFile.h"

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/util/Logger.h>

namespace e2d::audio {
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
		file_ = WavFile::construct(path);
		format_ = file_->get_format();

		auto al_format = get_internal_format(format_);

		// Prefetch data
		std::array<uint8_t, K_streaming_buffer_size> streaming_buffer_;
		for (int32_t i = 0; i < k_streaming_buffer_count; ++i) {
			// TODO: return readed number of samples
			auto samples_readed = file_->read_samples(streaming_buffer_);
			alCall(alBufferData, buffers_[i], al_format, streaming_buffer_.data(), samples_readed, format_.sample_rate);
		}

		return file_.get();
	}
}