#include "gfx_vk_query_pool.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "gfx_vk_util.h"

#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan
{
	QueryPool::QueryPool(Device& device, const QueryPoolCreateInfo& create_info) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device },
		max_query_count{ create_info.query_count } {

		vk::QueryPoolCreateInfo vk_create_info{};
		vk_create_info.queryType = query_type_to_vk(create_info.type);
		vk_create_info.queryCount = create_info.query_count;

		query_type = vk_create_info.queryType;

		vk::Result result = device.get_handle().createQueryPool(&vk_create_info, allocation_callbacks, &handle_);
	}

	QueryPool::QueryPool(QueryPool&& other) :
		Resource{ std::move(other) },
		device{ device },
		max_query_count{ std::exchange(other.max_query_count, {}) } {
	}

	QueryPool::~QueryPool() {
		device.get_handle().destroyQueryPool(handle_, allocation_callbacks);
	}

	void QueryPool::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void QueryPool::reset(uint32_t start_query, uint32_t query_count) {
		device.get_handle().resetQueryPool(handle_, start_query, query_count ? query_count : max_query_count);
	}

	void QueryPool::get_data(uint32_t query_index, void* data) const {
		switch (query_type) {
		case vk::QueryType::eOcclusion: {
			vk::Result result = device.get_handle()
				.getQueryPoolResults(handle_, query_index, 1u, sizeof(uint64_t), data, sizeof(uint64_t), vk::QueryResultFlagBits::e64);
		} break;
		case vk::QueryType::eTimestamp: {
			vk::Result result = device.get_handle()
				.getQueryPoolResults(handle_, query_index * 2u, 2u, sizeof(uint64_t) * 2u, data, sizeof(uint64_t), vk::QueryResultFlagBits::e64);
		} break;
		case vk::QueryType::ePipelineStatistics: {
			assert(false && "NOT IMPLEMENTED");
		} break;
		default:
			assert(false && "NOT IMPLEMENTED");
			break;
		}
	}

	void QueryPool::get_data(uint64_t first_query, uint32_t query_count, void* data) const {
		EDGE_PROFILE_ZONE_SCOPED;

		switch (query_type) {
		case vk::QueryType::eOcclusion:
		case vk::QueryType::eTimestamp: {
			vk::Result result = device.get_handle()
				.getQueryPoolResults(handle_, first_query, query_count, static_cast<uint32_t>(sizeof(uint64_t) * query_count), data, sizeof(uint64_t),
					vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);
		} break;
		case vk::QueryType::ePipelineStatistics: {
			assert(false && "NOT IMPLEMENTED");
		} break;
		default:
			assert(false && "NOT IMPLEMENTED");
			break;
		}
	}
}