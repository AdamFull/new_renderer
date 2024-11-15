#include "ALBuffer.h"

#include <AlUtil.h>

#include <AL/alext.h>

namespace engine
{
	namespace sfx
	{
		namespace openal
		{
			OpenALBuffer::OpenALBuffer(const BufferCreateInfo& create_info) :
				format{ AL_NONE },
				sample_rate{ create_info.format_info.sample_rate }
			{
				alCall(alGenBuffers, 1, &handle);

				bool float_32_support = alIsExtensionPresent("AL_EXT_FLOAT32");

				// Get this buffer format
				const auto& format_info = create_info.format_info;
				if (format_info.channels == 1)
				{
					if (format_info.sample_type == SampleType::eUInt8)
						format = AL_FORMAT_MONO8;
					else if (format_info.sample_type == SampleType::eInt16)
						format = AL_FORMAT_MONO16;
					else if (format_info.sample_type == SampleType::eFloat && float_32_support)
						format = AL_FORMAT_MONO_FLOAT32;
				}
				else if (format_info.channels == 2)
				{
					if (format_info.sample_type == SampleType::eUInt8)
						format = AL_FORMAT_STEREO8;
					else if (format_info.sample_type == SampleType::eInt16)
						format = AL_FORMAT_STEREO16;
					else if (format_info.sample_type == SampleType::eFloat && float_32_support)
						format = AL_FORMAT_STEREO_FLOAT32;
				}
			}

			OpenALBuffer::~OpenALBuffer()
			{
				alCall(alDeleteBuffers, 1, &handle);
			}

			uint64_t OpenALBuffer::Update(const void* data, uint64_t size)
			{
				alCall(alBufferData, handle, format, data, size, sample_rate);
			}

			uint32_t OpenALBuffer::GetHandle() const
			{
				return handle;
			}
		}
	}
}