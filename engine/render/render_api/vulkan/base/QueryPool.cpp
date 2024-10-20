#include <base/QueryPool.h>
#include <RenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			QueryPoolBuilder& QueryPoolBuilder::SetDebugName(const std::string& name)
			{
				debug_name = name;
				return *this;
			}

			QueryPoolBuilder& QueryPoolBuilder::SetFlags(vk::QueryPoolCreateFlags flags)
			{
				create_info.flags = flags;
				return *this;
			}

			QueryPoolBuilder& QueryPoolBuilder::SetQueryType(vk::QueryType query_type)
			{
				create_info.queryType = query_type;
				return *this;
			}

			QueryPoolBuilder& QueryPoolBuilder::SetQueryCount(uint32_t query_count)
			{
				create_info.queryCount = query_count;
				return *this;
			}

			QueryPoolBuilder& QueryPoolBuilder::SetPipelineStatisticsFlags(vk::QueryPipelineStatisticFlags pipeline_statistics)
			{
				create_info.pipelineStatistics = pipeline_statistics;
				return *this;
			}


			QueryPool QueryPoolBuilder::Build(RenderDevice& render_device) const
			{
				return QueryPool(render_device, *this);
			}

			std::unique_ptr<QueryPool> QueryPoolBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<QueryPool>(render_device, *this);
			}


			QueryPool::QueryPool(RenderDevice& render_device, const QueryPoolBuilder& builder) :
				VulkanResource{ nullptr },
				render_device{ render_device },
				max_query_count{ builder.create_info.queryCount }
			{
				handle = render_device.CreateQueryPool(builder.create_info, builder.debug_name);
			}

			QueryPool::QueryPool(QueryPool&& other) :
				VulkanResource{ std::move(other) },
				render_device{ render_device },
				max_query_count{ std::exchange(other.max_query_count, {}) }
			{
			}

			QueryPool::~QueryPool()
			{
				render_device.Destroy(handle);
			}

			void QueryPool::Reset(uint32_t first_query, uint32_t query_count)
			{
				render_device.ResetQueryPool(handle, first_query, query_count ? query_count : max_query_count);
			}

			vk::Result QueryPool::GetResults(uint32_t first_query, uint32_t num_queries, size_t result_bytes, void* results, vk::DeviceSize stride, vk::QueryResultFlags flags)
			{
				return render_device.GetQueryResults(handle, first_query, num_queries, result_bytes, results, stride, flags);
			}

			uint32_t QueryPool::GetQueryCount() const
			{
				return max_query_count;
			}
		}
	}
}