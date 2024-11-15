#pragma once

#include "Enums.h"

namespace engine
{
	namespace sfx
	{
		struct ContextCreateInfo
		{
			AudioApi audio_api{ AudioApi::eOpenAl };
		};

		struct AudioFormatInfo
		{
			uint8_t channels{ 1 };
			uint8_t bits_per_sample{ 16 };
			uint32_t sample_rate{ 48000 };
			SampleType sample_type{ SampleType::eUnknown };
		};

		struct BufferCreateInfo
		{
			AudioFormatInfo format_info{};
		};
	}
}