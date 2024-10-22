#include <VkCommandPool.h>
#include <VkDebug.h>

//#include <VkQueryPool.h>
#include <VkBuffer.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanCommandList::VulkanCommandList(VulkanCommandPool& command_pool, vk::CommandBufferLevel level) :
			VulkanResource{ nullptr, command_pool.GetAllocationCallbacks()},
			command_pool{ command_pool }
		{
			handle = command_pool.AllocateCommandBuffer(level);

			auto reset_mode = command_pool.GetResetMode();
			Reset(reset_mode);
		}

		VulkanCommandList::VulkanCommandList(VulkanCommandList&& other) :
			VulkanResource{ std::move(other) },
			command_pool{ other.command_pool }
		{
		}

		VulkanCommandList::~VulkanCommandList()
		{
			command_pool.FreeCommandBuffer(handle);
		}

		void VulkanCommandList::Reset(CommandPoolResetMode reset_mode)
		{
			if (reset_mode == CommandPoolResetMode::eResetIndividually)
			{
				handle.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
			}
		}

		void VulkanCommandList::Begin()
		{
			vk::CommandBufferBeginInfo begin_info{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
			vk::Result result = handle.begin(&begin_info);
		}

		void VulkanCommandList::End()
		{
			handle.end();
		}

		void VulkanCommandList::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
		{
			vk::Viewport viewport{ x, y, width, height, min_depth, max_depth };
			handle.setViewport(0u, 1u, &viewport);
		}

		void VulkanCommandList::SerScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			vk::Rect2D scissor{ { static_cast<int32_t>(x), static_cast<int32_t>(y) }, { width, height } };
			handle.setScissor(0u, 1u, &scissor);
		}

		void VulkanCommandList::BindPipeline(Pipeline* pipeline)
		{
			assert(false && "NOT IMPLEMENTED");
			//handle.bindPipeline(bind_point, pipeline);
		}

		void VulkanCommandList::BindDescriptorSet()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::BindDescriptorSets()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::PushConstants(RootSignature* root_signature, std::span<const char> constant_data)
		{
			assert(false && "NOT IMPLEMENTED");
			//handle.pushConstants(push_info.layout, push_info.stageFlags, push_info.offset, push_info.size, push_info.pValues);
		}

		void VulkanCommandList::BindIndexBuffer(Buffer* index_buffer, IndexType index_type, uint64_t offset)
		{
			vk::IndexType vk_index_type{ vk::IndexType::eUint32 };

			switch (index_type)
			{
			case gfx::IndexType::eUint16: vk_index_type = vk::IndexType::eUint16; break;
			case gfx::IndexType::eUint32: vk_index_type = vk::IndexType::eUint32; break;
			default:
				break;
			}

			auto* buffer = static_cast<VulkanBuffer*>(index_buffer);
			handle.bindIndexBuffer(buffer->GetHandle(), offset, vk_index_type);
		}

		void VulkanCommandList::BindVertexBuffers(uint32_t buffer_count, Buffer** vertex_buffers, const uint32_t* strides, const uint64_t* offsets)
		{
			std::array<vk::Buffer, 16ull> vk_vertex_buffers{};

			for (int32_t i = 0; i < buffer_count; ++i)
			{
				auto* buffer = static_cast<VulkanBuffer*>(vertex_buffers[i]);
				vk_vertex_buffers[i] = buffer->GetHandle();
			}

			handle.bindVertexBuffers(0u, buffer_count, vk_vertex_buffers.data(), offsets);
		}

		void VulkanCommandList::Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count)
		{
			handle.draw(vertex_count, instance_count, first_vertex, first_instance);
		}

		void VulkanCommandList::DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
		{
			handle.drawIndexed(index_count, instance_count, first_index, first_vertex, first_instance);
		}

		void VulkanCommandList::DrawIndirect()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::DrawIndirectIndexed()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::DrawIndirectIndexedCount()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::WriteTimestamp(QueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
		{
			handle.dispatch(group_x, group_y, group_z);
		}

		void VulkanCommandList::DispatchIndirect()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::PipelineBarrier()
		{
			assert(false && "NOT IMPLEMENTED");
			//handle.pipelineBarrier2KHR(dependency_info);
		}

		void VulkanCommandList::BeginQuery(QueryPool* query_pool)
		{
			assert(false && "NOT IMPLEMENTED");
			//handle.beginQuery();
		}

		void VulkanCommandList::EndQuery(QueryPool* query_pool)
		{
			assert(false && "NOT IMPLEMENTED");
			//handle.endQuery()
		}

		void VulkanCommandList::BeginRendering()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::EndRendering()
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::BeginMarker(const char* name, uint32_t color)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::InsertMarker(const char* name, uint32_t color)
		{
			assert(false && "NOT IMPLEMENTED");
		}

		void VulkanCommandList::EndMarker()
		{
			assert(false && "NOT IMPLEMENTED");
		}
	}
}