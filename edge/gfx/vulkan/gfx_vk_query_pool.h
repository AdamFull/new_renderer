#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class QueryPool final : public IQueryPool, public Resource<vk::QueryPool> {
	public:
		QueryPool(Device& device, const QueryPoolCreateInfo& create_info);
		QueryPool(QueryPool&& other);
		~QueryPool();

		QueryPool(const QueryPool&) = delete;
		QueryPool& operator=(const QueryPool&) = delete;
		QueryPool& operator=(QueryPool&&) = delete;

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