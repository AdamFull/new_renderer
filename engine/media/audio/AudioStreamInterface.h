#pragma once

#include "AudioSourceInterface.h"

namespace engine
{
	namespace media
	{
		class IAudioStream
		{
		public:
			virtual ~IAudioStream() = default;

			virtual void Update(IAudioSource& source) = 0;
		};
	}
}