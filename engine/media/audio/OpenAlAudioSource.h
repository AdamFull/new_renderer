#pragma once

#include "AudioSourceInterface.h"

namespace engine
{
	namespace media
	{
		class OpenAlAudioSource final : public IAudioSource
		{
		public:
			~OpenAlAudioSource() = default;
		};
	}
}