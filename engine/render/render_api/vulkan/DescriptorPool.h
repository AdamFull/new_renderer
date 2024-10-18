#pragma once

#include <engine/render/render_api/vulkan/DescriptorSet.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;

			class DescriptorPool final
			{
			public:
				DescriptorPool(RenderDevice& render_device);
				DescriptorPool(const DescriptorPool&) = delete;
				DescriptorPool(DescriptorPool&&);
				~DescriptorPool();

				DescriptorPool& operator=(const DescriptorPool&) = delete;
				DescriptorPool& operator=(DescriptorPool&&) = delete;

				void Reset();
				std::unique_ptr<DescriptorSet> Allocate(DescriptorSetLayout& layout, const std::string& debug_name = "");
				vk::Result Free(DescriptorSet& descriptor_set);
			private:
				void AllocateNewDescriptorPool(DescriptorSetLayout& layout);
			private:
				RenderDevice& render_device;

				struct DescriptorPoolBlock
				{
					// Max set count that can be allocated in one descriptor pool
					static constexpr size_t k_max_sets{ 50ull };

					// Initial number of descriptor count
					PoolSizes initial_sizes{};

					// Avaliable number of descriptors
					PoolSizes avaliable_sizes{};

					vk::DescriptorPool handle{};
					uint32_t allocated_count{ 0u };
				};

				uint32_t multiplicator{ 2u };
				std::vector<DescriptorPoolBlock> descriptor_pools{};
			};
		}
	}
}