#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class VulkanPipelineCache final : public IPipelineCache, public VulkanResource<vk::PipelineCache> {
	public:
		VulkanPipelineCache(Device& device, const PipelineCacheCreateInfo& create_info);
		~VulkanPipelineCache();

		VulkanPipelineCache(VulkanPipelineCache&) = delete;
		VulkanPipelineCache(VulkanPipelineCache&& other) noexcept;

		VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;
		VulkanPipelineCache& operator=(VulkanPipelineCache&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		void get_data(size_t& data_size, void* data_ptr) const override;
	private:
		Device& device;
	};
}