#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "IAudioInput.h"

#include <dr_wav.h>
#include <dr_mp3.h>
#include <dr_flac.h>

namespace e2d::audio {
	class AudioFile : public IAudioInput {
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

	class WavInput final : public AudioFile {
	public:
		WavInput();
		~WavInput() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioInput>;
	private:
		auto _read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t override;
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drwav, void(*)(drwav*)> wav_;
	};

	class Mp3Input final : public AudioFile {
	public:
		Mp3Input();
		~Mp3Input() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioInput>;

	private:
		auto _read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t override;
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drmp3, void(*)(drmp3*)> mp3_;
	};

	class FlacInput final : public AudioFile {
	public:
		FlacInput();
		~FlacInput() override = default;

		[[nodiscard]] static auto construct(std::span<const uint8_t> data) -> std::unique_ptr<IAudioInput>;

	private:
		auto _read_samples(std::span<uint8_t> out, size_t frames_to_read) -> size_t override;
		auto load_from_memory(std::span<const uint8_t> data) -> bool;
	private:
		std::unique_ptr<drflac, void(*)(drflac*)> flac_;
	};

	class MicrophoneInput final : public IAudioInput {
	public:
		MicrophoneInput();
		~MicrophoneInput() override;

		[[nodiscard]] static auto construct() -> std::unique_ptr<IAudioInput>;

		auto begin_recording() -> void;
		auto end_recording() -> void;

		auto begin_playback() -> void;
		auto end_playback() -> void;

		auto reset() -> void;

		auto get_format() const noexcept -> AudioFormat const& override;
		auto get_total_samples() const noexcept -> uint32_t override;
		auto get_samples_done() const noexcept -> uint32_t override;

		auto read_samples(std::span<uint8_t> out, const AudioFormat& out_format) -> uint32_t override;
	private:
		auto capture_start() -> bool;
		auto capture_stop() -> bool;

		auto initialize_resources() -> bool;

		struct _internal;
		std::unique_ptr<_internal> internal_;

		std::atomic_bool is_recording_{ false };
		std::atomic_bool is_listening_{ false };

		std::vector<uint8_t> recording_buffer_{};

		AudioFormat format_;
		uint32_t total_samples_{ 0u };
		uint32_t samples_processed_{ 0u };
	};
}