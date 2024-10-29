#pragma once

#include "VkDescriptorSetLayout.h"
#include <Base.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class VulkanDescriptorSet final : public DescriptorSet, public VulkanResource<vk::DescriptorSet>
			{
			public:
				friend class DescriptorPool;

				VulkanDescriptorSet();
				VulkanDescriptorSet(vk::DescriptorSet handle, const PoolSizes& sizes);

				VulkanDescriptorSet(const VulkanDescriptorSet&) = delete;

				VulkanDescriptorSet(VulkanDescriptorSet&& other);

				~VulkanDescriptorSet() = default;

				VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;
				VulkanDescriptorSet& operator=(VulkanDescriptorSet&&) = delete;
			private:
				PoolSizes used_sizes;
			};
		}
	}
}