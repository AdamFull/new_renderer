#include <VkQueryPool.h>
#include <VkDevice.h>

#include <VkUtil.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanQueryPool::VulkanQueryPool(Device& device, const QueryPoolCreateInfo& create_info) :
			VulkanResource{ nullptr, device.GetAllocationCallbacks() },
			device{ device },
			max_query_count{ create_info.query_count }
		{
			vk::QueryPoolCreateInfo vk_create_info{};
			vk_create_info.queryType = QueryTypeToVk(create_info.type);
			vk_create_info.queryCount = create_info.query_count;

			query_type = vk_create_info.queryType;

			vk::Result result = Handle(device).createQueryPool(&vk_create_info, allocation_callbacks, &handle);
		}

		VulkanQueryPool::VulkanQueryPool(VulkanQueryPool&& other) :
			VulkanResource{ std::move(other) },
			device{ device },
			max_query_count{ std::exchange(other.max_query_count, {}) }
		{
		}

		VulkanQueryPool::~VulkanQueryPool()
		{
			Handle(device).destroyQueryPool(handle, allocation_callbacks);
		}

		void VulkanQueryPool::Reset(uint32_t start_query, uint32_t query_count)
		{
			Handle(device).resetQueryPool(handle, start_query, query_count ? query_count : max_query_count);
		}

		void VulkanQueryPool::GetData(uint32_t query_index, void* data)
		{
			switch (query_type)
			{
			case vk::QueryType::eOcclusion:
			{
				vk::Result result = Handle(device).getQueryPoolResults(handle, query_index, 1u, sizeof(uint64_t), data, sizeof(uint64_t), vk::QueryResultFlagBits::e64);
			} break;
			case vk::QueryType::eTimestamp:
			{
				vk::Result result = Handle(device).getQueryPoolResults(handle, query_index * 2u, 2u, sizeof(uint64_t) * 2u, data, sizeof(uint64_t), vk::QueryResultFlagBits::e64);
			} break;
			case vk::QueryType::ePipelineStatistics:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			default:
				assert(false && "NOT IMPLEMENTED");
				break;
			}
		}

		void VulkanQueryPool::GetData(uint64_t first_query, uint32_t query_count, void* data)
		{
			switch (query_type)
			{
			case vk::QueryType::eOcclusion:
			case vk::QueryType::eTimestamp:
			{
				vk::Result result = Handle(device).getQueryPoolResults(handle, first_query, query_count, sizeof(uint64_t) * query_count, data, sizeof(uint64_t), vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);
			} break;
			case vk::QueryType::ePipelineStatistics:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			default:
				assert(false && "NOT IMPLEMENTED");
				break;
			}
		}

		uint32_t VulkanQueryPool::GetQueryCount() const
		{
			return max_query_count;
		}
	}
}