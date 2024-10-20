#pragma once

#include <base/DescriptorSetLayout.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class DescriptorSet final : public VulkanResource<vk::DescriptorSet>
			{
			public:
				friend class DescriptorPool;

				DescriptorSet() :
					VulkanResource{ nullptr }
				{
				}

				DescriptorSet(vk::DescriptorSet handle, const PoolSizes& sizes) :
					VulkanResource{ handle },
					used_sizes{ sizes }
				{
				}

				DescriptorSet(const DescriptorSet&) = delete;
				
				DescriptorSet(DescriptorSet&& other) :
					VulkanResource{ std::move(other) },
					used_sizes{ std::exchange(other.used_sizes, {}) }
				{
				}

				~DescriptorSet() = default;

				DescriptorSet& operator=(const DescriptorSet&) = delete;
				DescriptorSet& operator=(DescriptorSet&&) = delete;
			private:
				PoolSizes used_sizes;
			};
		}
	}
}