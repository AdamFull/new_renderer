#pragma once

#include <GFX.h>
#include <VkResource.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		class VulkanQueryPool final : public QueryPool, public VulkanResource<vk::QueryPool>
		{
		public:
			VulkanQueryPool(Device& device, const QueryPoolCreateInfo& create_info);
			VulkanQueryPool(VulkanQueryPool&& other);
			~VulkanQueryPool();

			VulkanQueryPool(const VulkanQueryPool&) = delete;
			VulkanQueryPool& operator=(const VulkanQueryPool&) = delete;
			VulkanQueryPool& operator=(VulkanQueryPool&&) = delete;

			void Reset(uint32_t start_query, uint32_t query_count) override;
			void GetData(uint32_t query_index, void* data) override;
			void GetData(uint64_t first_query, uint32_t query_count, void* data) override;

			uint32_t GetQueryCount() const;
		private:
			Device& device;

			vk::QueryType query_type{};
			uint32_t max_query_count{ 0u };
		};
	}
}