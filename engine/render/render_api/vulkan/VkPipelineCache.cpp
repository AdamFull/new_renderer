#include <VkPipelineCache.h>
#include <VkDevice.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanPipelineCache::VulkanPipelineCache(Device& device, vk::AllocationCallbacks* allocation_callbacks, const vk::PipelineCacheCreateInfo& create_info) :
			VulkanResource{ nullptr, allocation_callbacks },
			device{ device }
		{
			vk::Result result = Handle(device).createPipelineCache(&create_info, allocation_callbacks, &handle);
		}

		VulkanPipelineCache::~VulkanPipelineCache()
		{
			Handle(device).destroyPipelineCache(handle, allocation_callbacks);
		}

		VulkanPipelineCache::VulkanPipelineCache(VulkanPipelineCache&& other) noexcept :
			VulkanResource{ std::move(other) },
			device{ other.device }
		{
		}

		void VulkanPipelineCache::GetData(size_t& data_size, void* data_ptr)
		{
			vk::Result result = Handle(device).getPipelineCacheData(handle, &data_size, data_ptr);
		}
	}
}