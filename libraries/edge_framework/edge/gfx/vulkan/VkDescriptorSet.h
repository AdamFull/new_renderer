#pragma once

#include "VkDescriptorSetLayout.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class DescriptorSet final : public VulkanResource<vk::DescriptorSet> {
			public:
				friend class DescriptorPool;

				DescriptorSet();
				DescriptorSet(vk::DescriptorSet handle, const PoolSizes& sizes);

				DescriptorSet(const DescriptorSet&) = delete;

				DescriptorSet(DescriptorSet&& other);

				~DescriptorSet() = default;

				DescriptorSet& operator=(const DescriptorSet&) = delete;
				DescriptorSet& operator=(DescriptorSet&&) = delete;
			private:
				PoolSizes used_sizes;
			};
		}
	}
}