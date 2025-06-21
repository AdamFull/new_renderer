#pragma once

#include <memory>

#include "IAudioFile.h"

#include <dr_wav.h>
#include <dr_mp3.h>
#include <dr_flac.h>

namespace e2d::audio {
	class AudioFile : public IAudioFile {
	public:
		virtual ~AudioFile() override = default;

		auto get_format() const noexcept -> AudioFormat const& final override;
		auto get_total_samples() const noexcept -> uint32_t final override;
		auto get_samples_done() const noexcept -> uint32_t final override;

		auto read_samples(std::span<uint8_t> out, const AudioFormat& out_format) -> uint32_t final override;
	protected:
		virtual auto _read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t = 0;

		AudioFormat format_;
		uint32_t total_samples_{ 0u };
		uint32_t samples_processed_{ 0u };

		std::span<const uint8_t> file_bytes_{};
	};

	class WavFile final : public AudioFile {
	public:
		WavFile();
		~WavFile() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile>;
	private:
		auto _read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t override;
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drwav, void(*)(drwav*)> wav_;
	};

	class Mp3File final : public AudioFile {
	public:
		Mp3File();
		~Mp3File() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile>;

	private:
		auto _read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t override;
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drmp3, void(*)(drmp3*)> mp3_;
	};

	class FlacFile final : public AudioFile {
	public:
		FlacFile();
		~FlacFile() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioFile>;

	private:
		auto _read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t override;
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drflac, void(*)(drflac*)> flac_;
	};
}