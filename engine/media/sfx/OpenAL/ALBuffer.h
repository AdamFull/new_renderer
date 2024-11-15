#pragma once

#include <Base.h>

namespace engine
{
	namespace sfx
	{
		namespace openal
		{
			class OpenALBuffer final : public Buffer
			{
			public:
				OpenALBuffer(const BufferCreateInfo& create_info);
				~OpenALBuffer();

				uint64_t Update(const void* data, uint64_t size) override;
			private:
				uint32_t handle;
				uint32_t format;
				uint32_t sample_rate;
			};
		}
	}
}