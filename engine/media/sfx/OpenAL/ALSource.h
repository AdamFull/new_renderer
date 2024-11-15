#pragma once

#include <Base.h>

namespace engine
{
	namespace sfx
	{
		namespace openal
		{
			class OpenALSource final : public Source
			{
			public:
				OpenALSource();
				~OpenALSource();

				void Play();

				uint32_t GetHandle() const;

			private:
				uint32_t handle;
			};
		}
	}
}