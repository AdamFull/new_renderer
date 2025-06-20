#include "Util.h"

#include <AL/al.h>
#include <AL/alext.h>

namespace e2d::audio {
	auto get_internal_format(const AudioFormat& fmt) -> int32_t {
		ALenum al_format{ AL_FORMAT_STEREO8 };
		if (fmt.channels == 1 && fmt.bit_per_sample == 8)
			al_format = AL_FORMAT_MONO8;
		else if (fmt.channels == 1 && fmt.bit_per_sample == 16)
			al_format = AL_FORMAT_MONO16;
		else if (fmt.channels == 1 && fmt.bit_per_sample == 32)
			al_format = AL_FORMAT_MONO_FLOAT32;
		else if (fmt.channels == 2 && fmt.bit_per_sample == 8)
			al_format = AL_FORMAT_STEREO8;
		else if (fmt.channels == 2 && fmt.bit_per_sample == 16)
			al_format = AL_FORMAT_STEREO16;
		else if (fmt.channels == 2 && fmt.bit_per_sample == 32)
			al_format = AL_FORMAT_STEREO_FLOAT32;

		return al_format;
	}

	auto get_samples_per_channel(const AudioFormat& fmt, size_t buffer_size) -> size_t {
		auto bytes_per_sample = fmt.bit_per_sample / 8;
		return buffer_size / bytes_per_sample / fmt.channels;
	}
}