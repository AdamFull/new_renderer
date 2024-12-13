#include "ALContext.h"

#include "AlUtil.h"

#include "ALBuffer.h"
#include "ALSource.h"
#include "ALListener.h"

namespace engine
{
	namespace sfx
	{
		namespace openal
		{
			OpenALContext::OpenALContext()
			{
				device = alcOpenDevice(nullptr);

				if (!alcCall(alcCreateContext, context, device, device, nullptr) || !context)
				{
					EDGE_LOGE("Failed to create audio context.");
					return;
				}

				if (!alcCall(alcMakeContextCurrent, current_context, device, context) || current_context != ALC_TRUE)
				{
					EDGE_LOGE("Failed to make created audio context as current.");
					return;
				}

				if (!alCall(alDistanceModel, AL_INVERSE_DISTANCE_CLAMPED))
				{
					EDGE_LOGE("Failed to set distance model as inverse clamped. Using default one.");
				}
			}
			
			OpenALContext::~OpenALContext()
			{
				if (!alcCall(alcMakeContextCurrent, current_context, device, nullptr))
				{
					EDGE_LOGE("Failed to reset context.");
				}

				if (!alcCall(alcDestroyContext, device, context))
				{
					EDGE_LOGE("Failed to destroy context.");
					return;
				}

				bool closed{ false };
				alcCall(alcCloseDevice, closed, device, device);
			}

			void OpenALContext::Destroy(SFXResource* handle)
			{
				if (handle)
				{
					delete handle;
					handle = nullptr;
				}
			}

			Buffer* OpenALContext::CreateBuffer(const BufferCreateInfo& create_info)
			{
				return new OpenALBuffer(create_info);
			}

			Source* OpenALContext::CreateSource()
			{
				return new OpenALSource();
			}

			Listener* OpenALContext::CreateListener()
			{
				return new OpenALListener();
			}
		}
	}
}