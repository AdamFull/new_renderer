#pragma once

#include <base/DescriptorPool.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class DescriptorAllocator final
			{
			public:
				DescriptorAllocator(RenderDevice& render_device);
				DescriptorAllocator(const DescriptorAllocator&) = delete;
				DescriptorAllocator(DescriptorAllocator&&);
				~DescriptorAllocator();

				DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
				DescriptorAllocator& operator=(DescriptorAllocator&&) = delete;

				void Reset();
				DescriptorSet& Allocate(DescriptorSetLayout& layout, const std::string& debug_name = "");
			private:
				RenderDevice& render_device;

				// TODO: Do i even need to allocate memory here?
				uint32_t multiplicator{ 2u };
				std::vector<std::unique_ptr<DescriptorPool>> descriptor_pools{};
			};
		}
	}
}