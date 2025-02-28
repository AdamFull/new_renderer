#include "VkPipelineCache.h"
#include "VkDevice.h"
#include "VkDebug.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanPipelineCache::VulkanPipelineCache(Device& device, const PipelineCacheCreateInfo& create_info) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vk::PipelineCacheCreateInfo vk_create_info{};
				vk_create_info.pInitialData = create_info.data.data();
				vk_create_info.initialDataSize = create_info.data.size();

				vk::Result result = Handle(device).createPipelineCache(&vk_create_info, allocation_callbacks, &handle);
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

			void VulkanPipelineCache::SetResourceName(std::string_view name) const noexcept
			{
				auto& debug_utils = device.GetDebugUtils();
				debug_utils.SetDebugName(handle, name);
			}

			void VulkanPipelineCache::GetData(size_t& data_size, void* data_ptr)
			{
				vk::Result result = Handle(device).getPipelineCacheData(handle, &data_size, data_ptr);
			}
		}
	}
}
