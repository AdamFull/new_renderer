#pragma once

#include "VkResource.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class VulkanQueryPool final : public IQueryPool, public VulkanResource<vk::QueryPool> {
			public:
				VulkanQueryPool(Device& device, const QueryPoolCreateInfo& create_info);
				VulkanQueryPool(VulkanQueryPool&& other);
				~VulkanQueryPool();

				VulkanQueryPool(const VulkanQueryPool&) = delete;
				VulkanQueryPool& operator=(const VulkanQueryPool&) = delete;
				VulkanQueryPool& operator=(VulkanQueryPool&&) = delete;

				void set_resource_name(std::string_view name) const noexcept override;

				void reset(uint32_t start_query, uint32_t query_count) override;
				void get_data(uint32_t query_index, void* data) const override;
				void get_data(uint64_t first_query, uint32_t query_count, void* data) const override;

				auto get_query_count() const noexcept -> uint32_t {
					return max_query_count;
				}

				auto get_query_type() const noexcept -> vk::QueryType {
					return query_type;
				}
			private:
				Device& device;

				vk::QueryType query_type{};
				uint32_t max_query_count{ 0u };
			};
		}
	}
}