#pragma once

#include "AudioStreamInterface.h"
#include "AudioSourceInterface.h"

namespace engine
{
	namespace media
	{
		class IAudioDriver
		{
		public:
			virtual ~IAudioDriver() = default;

			virtual IAudioStream* OpenStream() = 0;
			virtual void CloseStream(IAudioStream* audio_stream) = 0;

			virtual IAudioSource* OpenSource() = 0;
			virtual void CloseSource(IAudioSource* audio_source) = 0;
		};
	}
}