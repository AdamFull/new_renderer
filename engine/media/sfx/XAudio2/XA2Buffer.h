#pragma once

#include <Base.h>

struct XAUDIO2_BUFFER;

namespace engine
{
	namespace sfx
	{
		namespace xa2
		{
			class XAudio2Buffer final : public Buffer
			{
			public:
				XAudio2Buffer();
				~XAudio2Buffer();

				uint64_t Update(const void* data, uint64_t size) override;
			private:
				XAUDIO2_BUFFER* handle{ nullptr };
			};
		}
	}
}