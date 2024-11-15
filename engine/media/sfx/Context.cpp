#include "Context.h"

#include "OpenAL/ALContext.h"

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
			default:
				break;
			}

			return nullptr;
		}
	}
}