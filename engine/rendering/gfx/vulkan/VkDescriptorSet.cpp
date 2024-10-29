#include "VkDescriptorSet.h"
#include "VkDevice.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanDescriptorSet::VulkanDescriptorSet() :
				VulkanResource{ nullptr, nullptr }
			{
			}

			VulkanDescriptorSet::VulkanDescriptorSet(vk::DescriptorSet handle, const PoolSizes& sizes) :
				VulkanResource{ handle, nullptr },
				used_sizes{ sizes }
			{
			}

			VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet&& other) :
				VulkanResource{ std::move(other) },
				used_sizes{ std::exchange(other.used_sizes, {}) }
			{
			}
		}
	}
}