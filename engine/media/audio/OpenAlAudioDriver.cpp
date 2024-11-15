#include "OpenAlAudioDriver.h"

#include "OpenAlUtil.h"

namespace engine
{
	namespace media
	{
		OpenAlAudioDrver::OpenAlAudioDrver()
		{
			device = alcOpenDevice(nullptr);

			if (!alcCall(alcCreateContext, context, device, device, nullptr) || !context)
			{
				log_error("Failed to create audio context.");
				return;
			}

			if (!alcCall(alcMakeContextCurrent, current_context, device, context) || current_context != ALC_TRUE)
			{
				log_error("Failed to make created audio context as current.");
				return;
			}

			if (!alCall(alDistanceModel, AL_INVERSE_DISTANCE_CLAMPED))
			{
				log_error("Failed to set distance model as inverse clamped. Using default one.");
			}
		}

		OpenAlAudioDrver::~OpenAlAudioDrver()
		{
			if (!alcCall(alcMakeContextCurrent, current_context, device, nullptr))
			{
				log_error("Failed to reset context.");
			}

			if (!alcCall(alcDestroyContext, device, context))
			{
				log_error("Failed to destroy context.");
				return;
			}

			bool closed{ false };
			alcCall(alcCloseDevice, closed, device, device);
		}

		IAudioStream* OpenAlAudioDrver::OpenStream()
		{
			return nullptr;
		}

		void OpenAlAudioDrver::CloseStream(IAudioStream* audio_stream)
		{

		}

		IAudioSource* OpenAlAudioDrver::OpenSource()
		{
			return nullptr;
		}

		void OpenAlAudioDrver::CloseSource(IAudioSource* audio_source)
		{

		}
	}
}