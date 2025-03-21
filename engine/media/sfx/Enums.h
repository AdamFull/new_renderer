#pragma once

#include <edge/util/utl_enum.hpp>

namespace engine
{
	namespace sfx
	{
		enum class AudioApi
		{
			eAuto,
			eOpenAl,
			eXAudio2
		};

		enum class ResourceType
		{
			eUnknown,
			eBuffer,
			eSource,
			eListener
		};

		enum class SampleType
		{
			eUnknown,
			eUInt8,
			eInt16,
			eFloat
		};
	}
}