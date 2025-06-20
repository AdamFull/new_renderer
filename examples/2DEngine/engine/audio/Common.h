#pragma once

#include <cstdint>

namespace e2d::audio {
	struct AudioFormat {
		uint32_t sample_rate{ 44100 };
		uint16_t channels{ 2 };
		uint16_t bit_per_sample{ 16 };

		auto get_number_of_frames_fit(size_t out_buffer_size) {
			auto bytes_per_sample = bit_per_sample / 8;
			return out_buffer_size / (bytes_per_sample * channels);
		}

		auto get_number_of_bytes(size_t num_of_frames) {
			auto bytes_per_sample = bit_per_sample / 8;
			return num_of_frames * bytes_per_sample * channels;
		}
	};
}