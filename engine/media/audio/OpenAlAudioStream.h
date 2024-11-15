#pragma once

#include "AudioStreamInterface.h"

namespace engine
{
	namespace media
	{
		class OpenAlAudioStream final : public IAudioStream
		{
		public:
			~OpenAlAudioStream() = default;
		};
	}
}