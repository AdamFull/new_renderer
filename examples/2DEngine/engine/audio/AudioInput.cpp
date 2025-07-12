#include "AudioInput.h"

#include <edge/util/Logger.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "BackendHelper.h"

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



	WavInput::WavInput() : wav_(nullptr,
		[](drwav* p) {
			if (p) {
				drwav_uninit(p);
				delete p;
			}
		})
	{}

	auto WavInput::construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioInput> {
		auto self = std::make_unique<WavInput>();
		self->load_from_memory(data);
		return self;
	}

	auto WavInput::_read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t {
		return drwav_read_pcm_frames_s16(wav_.get(), frames_to_read, reinterpret_cast<drwav_int16*>(out.data()));
	}

	auto WavInput::load_from_memory(std::span<const uint8_t> data) -> bool {
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



	Mp3Input::Mp3Input() : mp3_(nullptr,
		[](drmp3* p) {
			if (p) {
				drmp3_uninit(p);
				delete p;
			}
		})
	{}

	auto Mp3Input::construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioInput> {
		auto self = std::make_unique<Mp3Input>();
		self->load_from_memory(data);
		return self;
	}

	auto Mp3Input::_read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t {
		return drmp3_read_pcm_frames_s16(mp3_.get(), frames_to_read, reinterpret_cast<drmp3_int16*>(out.data()));
	}

	auto Mp3Input::load_from_memory(std::span<const uint8_t> data) -> bool {
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



	FlacInput::FlacInput() : flac_(nullptr,
		[](drflac* p) {
			if (p) {
				drflac_close(p);
			}
		})
	{}

	auto FlacInput::construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioInput> {
		auto self = std::make_unique<FlacInput>();
		self->load_from_memory(data);
		return self;
	}

	auto FlacInput::_read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t {
		return drflac_read_pcm_frames_s16(flac_.get(), frames_to_read, reinterpret_cast<drflac_int16*>(out.data()));
	}

	auto FlacInput::load_from_memory(std::span<const uint8_t> data) -> bool {
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



	struct MicrophoneInput::_internal {
		ALCdevice* dev_;
	};

	MicrophoneInput::MicrophoneInput() {
		format_.sample_rate = 44100u;
		format_.bit_per_sample = 16;
		format_.channels = 2;
	}

	MicrophoneInput::~MicrophoneInput() {
		if (internal_->dev_) {
			if (!alc_call_impl(alcCaptureStop, internal_->dev_, internal_->dev_)) {
				EDGE_LOGE("[MicrophoneInput] Failed to close device capture");
			}

			if (!alcCaptureCloseDevice(internal_->dev_)) {
				EDGE_LOGE("[MicrophoneInput] Failed to close device");
			}
		}
	}

	auto MicrophoneInput::construct() -> std::unique_ptr<IAudioInput> {
		auto self = std::make_unique<MicrophoneInput>();
		self->initialize_resources();
		return self;
	}

	auto MicrophoneInput::begin_recording() -> void {

	}

	auto MicrophoneInput::end_recording() -> void {
	}

	auto MicrophoneInput::begin_playback() -> void {
	}

	auto MicrophoneInput::end_playback() -> void {
	}

	auto MicrophoneInput::reset() -> void {
		if (is_recording_.load(std::memory_order_relaxed)) {
			is_recording_.store(false, std::memory_order_relaxed);
			is_listening_.store(false, std::memory_order_relaxed);
			recording_buffer_.clear();
			total_samples_ = 0u;
		}
	}

	auto MicrophoneInput::get_format() const noexcept -> AudioFormat const& {
		return format_;
	}

	auto MicrophoneInput::get_total_samples() const noexcept -> uint32_t {
		return 0;
	}

	auto MicrophoneInput::get_samples_done() const noexcept -> uint32_t {
		return 0;
	}

	auto MicrophoneInput::read_samples(std::span<uint8_t> out, const AudioFormat& out_format) -> uint32_t {
		if (!alc_call_impl(alcCaptureSamples, internal_->dev_, internal_->dev_, out.data(), 1024)) {
			EDGE_LOGW("[MicrophoneInput] Failed to read samples from device");
		}

		if (format_.channels == 2 && out_format.channels == 1 && out_format.bit_per_sample == 32) {
			stereo_pcm16_to_mono_float32(out);
		}

		return out.size();
	}

	auto MicrophoneInput::capture_start() -> bool {
		if (is_recording_.load(std::memory_order_relaxed)) {
			return false;
		}

		if (!alc_call_impl(alcCaptureStart, internal_->dev_, internal_->dev_)) {
			EDGE_LOGE("[MicrophoneInput] Failed start device capture");
			return false;
		}

		is_recording_.store(true, std::memory_order_relaxed);

		return true;
	}

	auto MicrophoneInput::capture_stop() -> bool {
		if (!is_recording_.load(std::memory_order_relaxed)) {
			return false;
		}

		if (!alc_call_impl(alcCaptureStop, internal_->dev_, internal_->dev_)) {
			EDGE_LOGE("[MicrophoneInput] Failed to close device capture");
			return false;
		}

		is_recording_.store(false, std::memory_order_relaxed);

		return true;
	}

	auto MicrophoneInput::initialize_resources() -> bool {
		internal_ = std::make_unique<_internal>();

		const ALCchar* devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
		const ALCchar* device = devices;
		while (*device) {
			printf("Capture device: %s\n", device);
			device += strlen(device) + 1;
		}

		internal_->dev_ = alcCaptureOpenDevice(NULL, format_.sample_rate, AL_FORMAT_STEREO16, 4096);
		if (!internal_->dev_) {
			EDGE_LOGE("[MicrophoneInput] Failed to open OpenAL device");
			return false;
		}
	}
}