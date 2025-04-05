#include "gfx_vk_pipeline_cache.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

namespace edge::gfx::vulkan
{
	PipelineCache::PipelineCache(Device& device, const PipelineCacheCreateInfo& create_info) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {
		vk::PipelineCacheCreateInfo vk_create_info{};
		vk_create_info.pInitialData = create_info.data.data();
		vk_create_info.initialDataSize = create_info.data.size();

		vk::Result result = device.get_handle().createPipelineCache(&vk_create_info, allocation_callbacks, &handle_);
	}

	PipelineCache::~PipelineCache() {
		device.get_handle().destroyPipelineCache(handle_, allocation_callbacks);
	}

	PipelineCache::PipelineCache(PipelineCache&& other) noexcept :
		Resource{ std::move(other) },
		device{ other.device } {
	}

	void PipelineCache::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void PipelineCache::get_data(size_t& data_size, void* data_ptr) const {
		vk::Result result = device.get_handle().getPipelineCacheData(handle_, &data_size, data_ptr);
	}
}
