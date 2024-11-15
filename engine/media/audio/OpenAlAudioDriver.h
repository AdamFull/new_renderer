#pragma once

#include "AudioDriverInterface.h"

#include <cstdint>

struct ALCdevice;
struct ALCcontext;

namespace engine
{
	namespace media
	{
		class OpenAlAudioDrver final : public IAudioDriver
		{
		public:
			OpenAlAudioDrver();
			~OpenAlAudioDrver();

			IAudioStream* OpenStream() override;
			void CloseStream(IAudioStream* audio_stream) override;

			IAudioSource* OpenSource() override;
			void CloseSource(IAudioSource* audio_source) override;

		private:
			ALCdevice* device{ nullptr };
			ALCcontext* context{ nullptr };
			int8_t current_context{};
		};
	}
}