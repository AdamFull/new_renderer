#include "ALSource.h"

#include "AlUtil.h"

namespace engine
{
	namespace sfx
	{
		namespace openal
		{
			OpenALSource::OpenALSource()
			{
				alCall(alGenSources, 1, &handle);
			}

			OpenALSource::~OpenALSource()
			{
				alCall(alDeleteSources, 1, &handle);
			}

			void OpenALSource::Play()
			{
				alCall(alSourcePlay, handle);
			}

			void OpenALSource::Rewind()
			{
				alCall(alSourceRewind, handle);
			}
		}
	}
}