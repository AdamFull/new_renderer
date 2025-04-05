#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class PipelineCache final : public IPipelineCache, public Resource<vk::PipelineCache> {
	public:
		PipelineCache(Device& device, const PipelineCacheCreateInfo& create_info);
		~PipelineCache();

		PipelineCache(PipelineCache&) = delete;
		PipelineCache(PipelineCache&& other) noexcept;

		PipelineCache& operator=(const PipelineCache&) = delete;
		PipelineCache& operator=(PipelineCache&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		void get_data(size_t& data_size, void* data_ptr) const override;
	private:
		Device& device;
	};
}