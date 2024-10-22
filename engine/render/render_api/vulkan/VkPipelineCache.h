#pragma once

#include <VkResource.h>
#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		class VulkanPipelineCache : public PipelineCache, public VulkanResource<vk::PipelineCache>
		{
		public:
			VulkanPipelineCache(Device& device, const PipelineCacheCreateInfo& create_info);
			~VulkanPipelineCache();

			VulkanPipelineCache(VulkanPipelineCache&) = delete;
			VulkanPipelineCache(VulkanPipelineCache&& other) noexcept;

			VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;
			VulkanPipelineCache& operator=(VulkanPipelineCache&&) = delete;

			void GetData(size_t& data_size, void* data_ptr) override;
		private:
			Device& device;
		};
	}
}