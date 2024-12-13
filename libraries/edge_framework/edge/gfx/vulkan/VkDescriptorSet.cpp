#include "VkDescriptorSet.h"
#include "VkDevice.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			DescriptorSet::DescriptorSet() :
				VulkanResource{ nullptr, nullptr }
			{
			}

			DescriptorSet::DescriptorSet(vk::DescriptorSet handle, const PoolSizes& sizes) :
				VulkanResource{ handle, nullptr },
				used_sizes{ sizes }
			{
			}

			DescriptorSet::DescriptorSet(DescriptorSet&& other) :
				VulkanResource{ std::move(other) },
				used_sizes{ std::exchange(other.used_sizes, {}) }
			{
			}
		}
	}
}