#include <VkCommandPool.h>
#include <VkDebug.h>

#include <VkQueue.h>
#include <VkQueryPool.h>
#include <VkBuffer.h>
#include <VkTexture.h>
#include <VkPipeline.h>

#include <VkUtil.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanCommandList::VulkanCommandList(VulkanCommandPool& command_pool, vk::CommandBufferLevel level) :
				VulkanResource{ nullptr, command_pool.GetAllocationCallbacks() },
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
				auto* vk_pipeline = static_cast<VulkanPipeline*>(pipeline);
				auto bind_point = PipelineTypeToVk(vk_pipeline->GetType());
				handle.bindPipeline(bind_point, vk_pipeline->GetHandle());
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

			void VulkanCommandList::DrawIndirect(const IndirectDrawCommand& draw_command)
			{
				auto* argument_buffer = static_cast<VulkanBuffer*>(draw_command.argument_buffer);
				auto* count_buffer = static_cast<VulkanBuffer*>(draw_command.count_buffer);

				if (draw_command.indexed_draw)
				{
					if (count_buffer)
					{
						handle.drawIndexedIndirectCount(argument_buffer->GetHandle(), draw_command.argument_offset, 
							count_buffer->GetHandle(), draw_command.count_offset, 
							draw_command.draw_count, draw_command.command_stride);
					}
					else
					{
						handle.drawIndexedIndirect(argument_buffer->GetHandle(), draw_command.argument_offset,
							draw_command.draw_count, draw_command.command_stride);
					}
				}
				else
				{
					if (count_buffer)
					{
						handle.drawIndirectCount(argument_buffer->GetHandle(), draw_command.argument_offset,
							count_buffer->GetHandle(), draw_command.count_offset,
							draw_command.draw_count, draw_command.command_stride);
					}
					else
					{
						handle.drawIndirect(argument_buffer->GetHandle(), draw_command.argument_offset,
							draw_command.draw_count, draw_command.command_stride);
					}
				}
			}

			void VulkanCommandList::WriteTimestamp(QueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag)
			{
				auto* pool = static_cast<VulkanQueryPool*>(query_pool);
				handle.writeTimestamp2KHR(StageFlagsToVk(stage_flag), pool->GetHandle(), pool_index);
			}

			void VulkanCommandList::Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
			{
				handle.dispatch(group_x, group_y, group_z);
			}

			void VulkanCommandList::DispatchIndirect(Buffer* argument_buffer, uint64_t argument_offset)
			{
				auto* vk_buffer = static_cast<VulkanBuffer*>(argument_buffer);
				handle.dispatchIndirect(vk_buffer->GetHandle(), argument_offset);
			}

			void VulkanCommandList::ResourceBarrier(const ResourceBarrierInfo& barrier)
			{
				std::array<vk::MemoryBarrier2KHR, 64ll> memory_barriers{};
				std::array<vk::BufferMemoryBarrier2KHR, 64ll> buffer_barriers{};
				std::array<vk::ImageMemoryBarrier2KHR, 64ll> image_barriers{};

				vk::DependencyInfoKHR vk_barrier{};
				vk_barrier.pMemoryBarriers = memory_barriers.data();
				vk_barrier.pBufferMemoryBarriers = buffer_barriers.data();
				vk_barrier.pImageMemoryBarriers = image_barriers.data();

				// Parse memory barriers
				for (int32_t i = 0; i < barrier.memory_barrier_count; ++i)
				{
					auto& memory_barrier = barrier.memory_barriers[i];

					auto& vk_memory_barrier = memory_barriers[vk_barrier.memoryBarrierCount++];
					vk_memory_barrier.srcAccessMask = AccessFlagsToVk(memory_barrier.src_state.access);
					vk_memory_barrier.srcStageMask = StageFlagsToVk(memory_barrier.src_state.stage);
					vk_memory_barrier.dstAccessMask = AccessFlagsToVk(memory_barrier.dst_state.access);
					vk_memory_barrier.dstStageMask = StageFlagsToVk(memory_barrier.dst_state.stage);
				}

				// Parse buffer barriers
				for (int32_t i = 0; i < barrier.buffer_barrier_count; ++i)
				{
					auto& buffer_barrier = barrier.buffer_barriers[i];
					auto* buffer = static_cast<VulkanBuffer*>(buffer_barrier.buffer);

					auto& vk_buffer_barrier = buffer_barriers[vk_barrier.bufferMemoryBarrierCount++];
					vk_buffer_barrier.buffer = buffer->GetHandle();

					if (buffer_barrier.range.offset != 0ull && buffer_barrier.range.size != 0ull)
					{
						vk_buffer_barrier.offset = buffer_barrier.range.offset;
						vk_buffer_barrier.size = buffer_barrier.range.size;
					}
					else
					{
						vk_buffer_barrier.size = buffer->GetSize();
					}

					vk_buffer_barrier.srcAccessMask = AccessFlagsToVk(buffer_barrier.src_state.access);
					vk_buffer_barrier.srcStageMask = StageFlagsToVk(buffer_barrier.src_state.stage);
					vk_buffer_barrier.dstAccessMask = AccessFlagsToVk(buffer_barrier.dst_state.access);
					vk_buffer_barrier.dstStageMask = StageFlagsToVk(buffer_barrier.dst_state.stage);

					if (buffer_barrier.src_state.queue && buffer_barrier.dst_state.queue)
					{
						const auto* src_queue = static_cast<const VulkanQueue*>(buffer_barrier.src_state.queue);
						const auto* dst_queue = static_cast<const VulkanQueue*>(buffer_barrier.dst_state.queue);
						
						vk_buffer_barrier.srcQueueFamilyIndex = src_queue->GetFamilyIndex();
						vk_buffer_barrier.dstQueueFamilyIndex = dst_queue->GetFamilyIndex();
					}
				}

				// Parse image barriers
				for (int32_t i = 0; i < barrier.texture_barrier_count; ++i)
				{
					auto& texture_barrier = barrier.texture_barriers[i];
					auto* texture = static_cast<VulkanTexture*>(texture_barrier.texture);

					auto& vk_image_barrier = image_barriers[vk_barrier.imageMemoryBarrierCount++];

					auto& image = texture->GetImage();
					vk_image_barrier.image = image.GetHandle();

					auto& subresource = texture->GetSubresource(texture_barrier.subresource_index);
					vk_image_barrier.subresourceRange = subresource.GetSubresourceRange();

					vk_image_barrier.oldLayout = ImageLayoutToVk(texture_barrier.src_layout);
					assert(vk_image_barrier.oldLayout == subresource.GetImageLayout() && "Old layout is not correct.");
					vk_image_barrier.srcAccessMask = AccessFlagsToVk(texture_barrier.src_state.access);
					vk_image_barrier.srcStageMask = StageFlagsToVk(texture_barrier.src_state.stage);

					vk_image_barrier.newLayout = ImageLayoutToVk(texture_barrier.dst_layout);
					vk_image_barrier.dstAccessMask = AccessFlagsToVk(texture_barrier.dst_state.access);
					vk_image_barrier.dstStageMask = StageFlagsToVk(texture_barrier.dst_state.stage);

					if (texture_barrier.src_state.queue && texture_barrier.dst_state.queue)
					{
						const auto* src_queue = static_cast<const VulkanQueue*>(texture_barrier.src_state.queue);
						const auto* dst_queue = static_cast<const VulkanQueue*>(texture_barrier.dst_state.queue);

						vk_image_barrier.srcQueueFamilyIndex = src_queue->GetFamilyIndex();
						vk_image_barrier.dstQueueFamilyIndex = dst_queue->GetFamilyIndex();
					}
				}

				handle.pipelineBarrier2KHR(vk_barrier);
			}

			void VulkanCommandList::BeginQuery(QueryPool* query_pool, uint32_t query)
			{
				auto* vk_query_pool = static_cast<VulkanQueryPool*>(query_pool);
				handle.beginQuery(vk_query_pool->GetHandle(), query, {});
			}

			void VulkanCommandList::EndQuery(QueryPool* query_pool, uint32_t query)
			{
				auto* vk_query_pool = static_cast<VulkanQueryPool*>(query_pool);
				handle.endQuery(vk_query_pool->GetHandle(), query);
			}

			void VulkanCommandList::BeginRendering()
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void VulkanCommandList::EndRendering()
			{
				handle.endRenderingKHR();
			}

			void VulkanCommandList::BeginMarker(const char* name, uint32_t color)
			{
				auto& debug_utils = command_pool.GetDebugUtils();
				debug_utils.BeginLabel(handle, name, color);
			}

			void VulkanCommandList::InsertMarker(const char* name, uint32_t color)
			{
				auto& debug_utils = command_pool.GetDebugUtils();
				debug_utils.InsertLabel(handle, name, color);
			}

			void VulkanCommandList::EndMarker()
			{
				auto& debug_utils = command_pool.GetDebugUtils();
				debug_utils.EndLabel(handle);
			}
		}
	}
}