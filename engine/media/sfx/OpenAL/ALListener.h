#pragma once

#include <Base.h>

namespace engine
{
	namespace sfx
	{
		namespace openal
		{
			class OpenALListener final : public Listener
			{
			public:
				OpenALListener();
				~OpenALListener();
			};
		}
	}
}