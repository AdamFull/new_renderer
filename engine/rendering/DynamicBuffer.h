#pragma once

#include <engine/rendering/gfx/Structures.h>
#include "DynamicBufferInterface.h"

namespace engine
{
	namespace gfx
	{
		class ResourceContext;
	}

	namespace render
	{
		class DynamicBuffer final : public IDynamicBuffer
		{
		public:
			struct InsertionRange
			{
				uint64_t offset{ 0ull };
				uint64_t size{ 0ull };
			};

			DynamicBuffer(gfx::ResourceContext& resource_context, uint64_t element_size, uint64_t capacity = 64u, gfx::BufferUsageFlags flags = {});
			~DynamicBuffer();

			void Flush() override;
		protected:
			void TryGrow(uint64_t offset, uint64_t size);
			uint64_t CalculateOffset(uint64_t index) override;
			void InsertBytes(const void* data, uint64_t size, uint64_t offset) override;
			void* GetBytes(uint64_t offset) override;
		private:
			gfx::ResourceContext& resource_context;

			uint64_t element_size{ 0ull };
			gfx::BufferUsageFlags usage_flags{};

			gfx::Buffer* buffer{ nullptr };
			std::vector<uint8_t> memory{};

			std::vector<InsertionRange> insertion_ranges{};
		};
	}
}