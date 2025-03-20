#include "VkPipelineCache.h"
#include "VkDevice.h"
#include "VkDebug.h"

namespace edge::gfx::vulkan
{
	VulkanPipelineCache::VulkanPipelineCache(Device& device, const PipelineCacheCreateInfo& create_info) :
		VulkanResource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {
		vk::PipelineCacheCreateInfo vk_create_info{};
		vk_create_info.pInitialData = create_info.data.data();
		vk_create_info.initialDataSize = create_info.data.size();

		vk::Result result = device.get_handle().createPipelineCache(&vk_create_info, allocation_callbacks, &handle_);
	}

	VulkanPipelineCache::~VulkanPipelineCache() {
		device.get_handle().destroyPipelineCache(handle_, allocation_callbacks);
	}

	VulkanPipelineCache::VulkanPipelineCache(VulkanPipelineCache&& other) noexcept :
		VulkanResource{ std::move(other) },
		device{ other.device } {
	}

	void VulkanPipelineCache::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void VulkanPipelineCache::get_data(size_t& data_size, void* data_ptr) const {
		vk::Result result = device.get_handle().getPipelineCacheData(handle_, &data_size, data_ptr);
	}
}
