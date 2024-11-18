#include "Context.h"

#include "OpenAL/ALContext.h"
#include "XAudio2/XA2Context.h"

namespace engine
{
	namespace sfx
	{
		std::unique_ptr<Context> CreateContext(const ContextCreateInfo& create_info)
		{
			switch (create_info.audio_api)
			{
			case AudioApi::eOpenAl:
				return std::make_unique<openal::OpenALContext>();
				break;
			case AudioApi::eXAudio2:
				return std::make_unique<xa2::XAudio2Context>();
				break;
			default:
				break;
			}

			return nullptr;
		}
	}
}