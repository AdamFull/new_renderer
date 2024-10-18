#pragma once

#include <engine/render/render_api/vulkan/DescriptorSetLayout.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class DescriptorPool;

			class DescriptorSet final : public VulkanResource<vk::DescriptorSet>
			{
			public:
				friend class DescriptorPool;

				DescriptorSet(DescriptorPool& descriptor_pool, vk::DescriptorSet handle, size_t pool_index, const PoolSizes& used_sizes);
				DescriptorSet(const DescriptorSet&) = delete;
				DescriptorSet(DescriptorSet&& other);

				~DescriptorSet();

				DescriptorSet& operator=(const DescriptorSet&) = delete;
				DescriptorSet& operator=(DescriptorSet&&) = delete;
			private:
				DescriptorPool& descriptor_pool;

				size_t pool_index{ ~0ull };
				PoolSizes used_sizes;
			};
		}
	}
}