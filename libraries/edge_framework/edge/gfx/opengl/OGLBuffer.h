#pragma once

#include "OGLResource.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace opengl
		{
			class OpenGLBuffer final : public Buffer, public OpenGLResource
			{
			public:
				OpenGLBuffer(const BufferCreateInfo& create_info);
				~OpenGLBuffer();

				void SetResourceName(std::string_view name) override;

				uint8_t* Map() override;

				void Unmap() override;

				uint64_t Update(const void* data, uint64_t size, uint64_t offset = 0ull) override;

				uint64_t GetSize() const override;
				uint64_t GetAddress() const override;

				BufferUsageFlags GetUsageFlags() const override;
			private:
				BufferUsageFlags usage_flags{};
				size_t size{ 0ull };
				size_t suballocation_count{ 0ull };

				uint32_t target{ 0u };

				uint8_t* mapped_memory{ nullptr };
			};
		}
	}
}