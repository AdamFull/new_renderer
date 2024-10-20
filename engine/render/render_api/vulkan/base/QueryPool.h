#pragma once

#include <VkResource.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;
			class QueryPool;

			class QueryPoolBuilder final
			{
			public:
				std::string debug_name{};
				vk::QueryPoolCreateInfo create_info{};

				QueryPoolBuilder& SetDebugName(const std::string& name);
				QueryPoolBuilder& SetFlags(vk::QueryPoolCreateFlags flags);
				QueryPoolBuilder& SetQueryType(vk::QueryType query_type);
				QueryPoolBuilder& SetQueryCount(uint32_t query_count);
				QueryPoolBuilder& SetPipelineStatisticsFlags(vk::QueryPipelineStatisticFlags pipeline_statistics);

				QueryPool Build(RenderDevice& render_device) const;
				std::unique_ptr<QueryPool> BuildUnique(RenderDevice& render_device) const;
			};

			class QueryPool final : public VulkanResource<vk::QueryPool>
			{
			public:
				QueryPool(RenderDevice& render_device, const QueryPoolBuilder& builder);
				QueryPool(QueryPool&& other);
				~QueryPool();

				QueryPool(const QueryPool&) = delete;
				QueryPool& operator=(const QueryPool&) = delete;
				QueryPool& operator=(QueryPool&&) = delete;

				void Reset(uint32_t first_query, uint32_t query_count = 0u);
				vk::Result GetResults(uint32_t first_query, uint32_t num_queries, size_t result_bytes, void* results, vk::DeviceSize stride, vk::QueryResultFlags flags);

				uint32_t GetQueryCount() const;
			private:
				RenderDevice& render_device;

				uint32_t max_query_count{ 0u };
			};
		}
	}
}