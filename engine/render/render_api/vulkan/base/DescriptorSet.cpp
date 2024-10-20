#include <base/DescriptorPool.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			DescriptorSet::DescriptorSet(DescriptorPool& descriptor_pool, vk::DescriptorSet handle, const PoolSizes& used_sizes) :
				VulkanResource{ handle },
				descriptor_pool{ descriptor_pool },
				pool_index{ pool_index },
				used_sizes{ used_sizes }
			{
			}


			DescriptorSet::DescriptorSet(DescriptorSet&& other) :
				VulkanResource{ std::move(other) },
				descriptor_pool{ other.descriptor_pool }
			{
			}

			DescriptorSet::~DescriptorSet()
			{
				//descriptor_pool.Free(*this);
			}
		}
	}
}