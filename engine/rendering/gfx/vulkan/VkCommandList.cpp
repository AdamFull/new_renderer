#include "VkCommandPool.h"
#include "VkDebug.h"

#include "VkQueue.h"
#include "VkQueryPool.h"
#include "VkBuffer.h"
#include "VkTexture.h"
#include "VkPipeline.h"
#include "VkRootSignature.h"
#include "VkDescriptorSet.h"
#include "VkResourceDescriptor.h"

#include <Util.h>
#include "VkUtil.h"

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

			void VulkanCommandList::BindDescriptors(const BindDescriptorsInfo& bind_info)
			{
				auto* vk_root_signature = static_cast<VulkanRootSignature*>(bind_info.root_signature);

				uint32_t first_descriptor_set{ ~0u };
				uint32_t descriptor_set_count{ 0u };
				std::array<vk::DescriptorSet, k_max_descriptor_count> descriptor_sets{};
				for (int32_t i = 0; i < bind_info.resource_descriptors_count; ++i)
				{
					auto* resource_descriptor = bind_info.resource_descriptors[i];
					if (resource_descriptor)
					{
						auto* vk_resource_descriptor = static_cast<VulkanResourceDescriptor*>(resource_descriptor);
						auto first_set = vk_resource_descriptor->GetFirstSet();
						auto set_count = vk_resource_descriptor->GetSetCount();
						auto& sets = vk_resource_descriptor->GetDescriptorSets();

						// TODO: check that descriptor ranges is not overlapping
						for (int32_t j = first_set; j < first_set + set_count; ++j)
						{
							descriptor_sets[j] = sets[j];
							first_descriptor_set = std::min(first_descriptor_set, (uint32_t)j);
							descriptor_set_count++;
						}
					}
				}

				auto& pipeline_layout = vk_root_signature->GetPipelineLayout();
				handle.bindDescriptorSets(
					PipelineTypeToVk(bind_info.pipeline_type), 
					pipeline_layout.GetHandle(), 
					first_descriptor_set, 
					descriptor_set_count, 
					descriptor_sets.data() + first_descriptor_set, 
					0u, nullptr);
			}

			void VulkanCommandList::PushConstants(Shader* shader, RootSignature* root_signature, std::span<const char> constant_data)
			{
				auto* vk_root_signature = static_cast<VulkanRootSignature*>(root_signature);
				auto& pipeline_layout = vk_root_signature->GetPipelineLayout();

				auto& reflection = shader->GetReflection();
				handle.pushConstants(pipeline_layout.GetHandle(), ShaderStageFlagsToVk(reflection.constants.stages), 0u, constant_data.size(), constant_data.data());
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

			//void VulkanCommandList::WriteTimestamp(QueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag)
			//{
			//	auto* pool = static_cast<VulkanQueryPool*>(query_pool);
			//	handle.writeTimestamp2KHR(StageFlagsToVk(stage_flag), pool->GetHandle(), pool_index);
			//}

			void VulkanCommandList::Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
			{
				handle.dispatch(group_x, group_y, group_z);
			}

			void VulkanCommandList::DispatchIndirect(Buffer* argument_buffer, uint64_t argument_offset)
			{
				auto* vk_buffer = static_cast<VulkanBuffer*>(argument_buffer);
				handle.dispatchIndirect(vk_buffer->GetHandle(), argument_offset);
			}

			void VulkanCommandList::CopyBuffer(const CopyBufferInfo& copy_info)
			{
				std::array<vk::BufferCopy2KHR, 64ull> regions{};
				vk::CopyBufferInfo2KHR vk_copy_info{};
				vk_copy_info.pRegions = regions.data();

				auto* src_buffer = static_cast<VulkanBuffer*>(copy_info.src_buffer);
				vk_copy_info.srcBuffer = src_buffer->GetHandle();
				auto* dst_buffer = static_cast<VulkanBuffer*>(copy_info.dst_buffer);
				vk_copy_info.dstBuffer = dst_buffer->GetHandle();

				for (int32_t i = 0; i < copy_info.region_count; ++i)
				{
					auto& region = copy_info.regions[i];
					auto& vk_region = regions[vk_copy_info.regionCount++];
					vk_region.srcOffset = region.src_offset;
					vk_region.dstOffset = region.dst_offset;
					vk_region.size = region.size;
				}

				handle.copyBuffer2KHR(vk_copy_info);
			}

			void VulkanCommandList::CopyTexture(const CopyTextureInfo& copy_info)
			{
				std::array<vk::BufferImageCopy2KHR, 64ull> regions{};
				vk::CopyBufferToImageInfo2KHR vk_copy_info{};
				vk_copy_info.pRegions = regions.data();

				auto* src_buffer = static_cast<VulkanBuffer*>(copy_info.src_buffer);
				vk_copy_info.srcBuffer = src_buffer->GetHandle();

				auto* dst_texture = static_cast<VulkanTexture*>(copy_info.dst_texture);
				auto& dst_image = dst_texture->GetImage();
				auto& subresource = dst_texture->GetSubresource();
				vk_copy_info.dstImage = dst_image.GetHandle();

				// TODO: As i think it will be transfer dst in any case
				vk_copy_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;

				for (int32_t i = 0; i < copy_info.copy_region_count; ++i)
				{
					auto& region = copy_info.copy_regions[i];
					auto& vk_region = regions[vk_copy_info.regionCount++];

					vk_region.bufferOffset = region.buffer_offset;
					vk_region.imageSubresource.aspectMask = subresource.GetAspectMask();
					vk_region.imageSubresource.mipLevel = region.level;
					vk_region.imageSubresource.baseArrayLayer = region.layer;
					vk_region.imageSubresource.layerCount = region.layer_count;

					vk_region.imageOffset.x = region.offset_x;
					vk_region.imageOffset.y = region.offset_y;
					vk_region.imageOffset.z = region.offset_z;

					vk_region.imageExtent.width = region.size_x;
					vk_region.imageExtent.height = region.size_y;
					vk_region.imageExtent.depth = region.size_z;
				}

				handle.copyBufferToImage2KHR(vk_copy_info);
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
					vk_memory_barrier.srcAccessMask = ResourceStateToVkAccessFlags(memory_barrier.src_state);
					vk_memory_barrier.srcStageMask = ResourceStateToVkStageFlags(memory_barrier.src_state);
					vk_memory_barrier.dstAccessMask = ResourceStateToVkAccessFlags(memory_barrier.dst_state);
					vk_memory_barrier.dstStageMask = ResourceStateToVkStageFlags(memory_barrier.dst_state);
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

					vk_buffer_barrier.srcAccessMask = ResourceStateToVkAccessFlags(buffer_barrier.src_state);
					vk_buffer_barrier.srcStageMask = ResourceStateToVkStageFlags(buffer_barrier.src_state);
					vk_buffer_barrier.dstAccessMask = ResourceStateToVkAccessFlags(buffer_barrier.dst_state);
					vk_buffer_barrier.dstStageMask = ResourceStateToVkStageFlags(buffer_barrier.dst_state);

					// TODO: NOT IMPLEMENTED TRANSFER OWNERSHIP
					//if (buffer_barrier.src_state.queue && buffer_barrier.dst_state.queue)
					//{
					//	const auto* src_queue = static_cast<const VulkanQueue*>(buffer_barrier.src_state.queue);
					//	const auto* dst_queue = static_cast<const VulkanQueue*>(buffer_barrier.dst_state.queue);
					//	
					//	vk_buffer_barrier.srcQueueFamilyIndex = src_queue->GetFamilyIndex();
					//	vk_buffer_barrier.dstQueueFamilyIndex = dst_queue->GetFamilyIndex();
					//}
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

					vk_image_barrier.oldLayout = ResourceStateToVkImageLayout(texture_barrier.src_state);
					//assert(vk_image_barrier.oldLayout == subresource.GetImageLayout() && "Old layout is not correct."); 
					vk_image_barrier.srcAccessMask = ResourceStateToVkAccessFlags(texture_barrier.src_state);
					vk_image_barrier.srcStageMask = ResourceStateToVkStageFlags(texture_barrier.src_state);

					vk_image_barrier.newLayout = ResourceStateToVkImageLayout(texture_barrier.dst_state);
					vk_image_barrier.dstAccessMask = ResourceStateToVkAccessFlags(texture_barrier.dst_state);
					vk_image_barrier.dstStageMask = ResourceStateToVkStageFlags(texture_barrier.dst_state);

					// TODO: NOT IMPLEMENTED TRANSFER OWNERSHIP
					//if (src_state.queue && dst_state.queue)
					//{
					//	const auto* src_queue = static_cast<const VulkanQueue*>(src_state.queue);
					//	const auto* dst_queue = static_cast<const VulkanQueue*>(dst_state.queue);
					//
					//	vk_image_barrier.srcQueueFamilyIndex = src_queue->GetFamilyIndex();
					//	vk_image_barrier.dstQueueFamilyIndex = dst_queue->GetFamilyIndex();
					//}
				}

				handle.pipelineBarrier2KHR(vk_barrier);
			}

			void VulkanCommandList::ResetQuery(QueryPool* query_pool, uint32_t first_query, uint32_t query_count)
			{
				auto* vk_query_pool = static_cast<VulkanQueryPool*>(query_pool);
				handle.resetQueryPool(vk_query_pool->GetHandle(), first_query, query_count);
			}

			void VulkanCommandList::BeginQuery(QueryPool* query_pool, uint32_t query)
			{
				auto* vk_query_pool = static_cast<VulkanQueryPool*>(query_pool);
				
				switch (vk_query_pool->GetQueryType())
				{
				case vk::QueryType::eTimestamp: 
				{
					handle.writeTimestamp2KHR(vk::PipelineStageFlagBits2KHR::eTopOfPipe, vk_query_pool->GetHandle(), query);
				} break;
				case vk::QueryType::eOcclusion:
				case vk::QueryType::ePipelineStatistics:
				{
					handle.beginQuery(vk_query_pool->GetHandle(), query, {});
				};
				default:
				{
					assert(false && "UNSUPPORTED QUERY TYPE");
				} break;
				}
			}

			void VulkanCommandList::EndQuery(QueryPool* query_pool, uint32_t query)
			{
				auto* vk_query_pool = static_cast<VulkanQueryPool*>(query_pool);

				switch (vk_query_pool->GetQueryType())
				{
				case vk::QueryType::eTimestamp:
				{
					handle.writeTimestamp2KHR(vk::PipelineStageFlagBits2KHR::eBottomOfPipe, vk_query_pool->GetHandle(), query);
				} break;
				case vk::QueryType::eOcclusion:
				case vk::QueryType::ePipelineStatistics:
				{
					handle.endQuery(vk_query_pool->GetHandle(), query);
				};
				default:
				{
					assert(false && "UNSUPPORTED QUERY TYPE");
				} break;
				}
			}

			void VulkanCommandList::BeginRendering(const RenderingInfo& rendering_info)
			{
				vk::RenderingInfoKHR begin_rendering_info{};
				begin_rendering_info.renderArea.offset.x = rendering_info.offset_x;
				begin_rendering_info.renderArea.offset.y = rendering_info.offset_y;
				begin_rendering_info.renderArea.extent.width = rendering_info.width;
				begin_rendering_info.renderArea.extent.height = rendering_info.height;

				begin_rendering_info.layerCount = rendering_info.layer_count;

				std::array<vk::RenderingAttachmentInfo, 8ull> color_attachments{};
				begin_rendering_info.pColorAttachments = color_attachments.data();

				for (int32_t i = 0; i < rendering_info.attachment_count; ++i)
				{
					auto& attachment = rendering_info.attachments[i];
					auto& vk_attachment = color_attachments[begin_rendering_info.colorAttachmentCount++];

					assert(attachment.texture);
					auto* texture = static_cast<VulkanTexture*>(attachment.texture);
					auto& subresource = texture->GetSubresource();

					vk_attachment.imageView = subresource.GetHandle();
					vk_attachment.imageLayout = ResourceStateToVkImageLayout(attachment.texture_state);
					vk_attachment.loadOp = LoadOpToVk(attachment.load_op);
					vk_attachment.storeOp = StoreOpToVk(attachment.store_op);

					vk_attachment.clearValue.color = attachment.clear_color.color;
				}

				vk::RenderingAttachmentInfo depth_attachment{};
				if (rendering_info.depth_attachment.texture)
				{
					auto* texture = static_cast<VulkanTexture*>(rendering_info.depth_attachment.texture);
					auto& subresource = texture->GetSubresource();

					depth_attachment.imageView = subresource.GetHandle();
					depth_attachment.imageLayout = ResourceStateToVkImageLayout(rendering_info.depth_attachment.texture_state);
					depth_attachment.loadOp = LoadOpToVk(rendering_info.depth_attachment.load_op);
					depth_attachment.storeOp = StoreOpToVk(rendering_info.depth_attachment.store_op);

					depth_attachment.clearValue.depthStencil.depth = rendering_info.depth_attachment.clear_color.depth;
					depth_attachment.clearValue.depthStencil.stencil = rendering_info.depth_attachment.clear_color.stencil;

					begin_rendering_info.pDepthAttachment = &depth_attachment;
				}

				vk::RenderingAttachmentInfo stencil_attachment{};
				if (rendering_info.stencil_attachment.texture)
				{
					auto* texture = static_cast<VulkanTexture*>(rendering_info.stencil_attachment.texture);
					auto& subresource = texture->GetSubresource();

					stencil_attachment.imageView = subresource.GetHandle();
					stencil_attachment.imageLayout = ResourceStateToVkImageLayout(rendering_info.stencil_attachment.texture_state);
					stencil_attachment.loadOp = LoadOpToVk(rendering_info.stencil_attachment.load_op);
					stencil_attachment.storeOp = StoreOpToVk(rendering_info.stencil_attachment.store_op);

					stencil_attachment.clearValue.depthStencil.depth = rendering_info.stencil_attachment.clear_color.depth;
					stencil_attachment.clearValue.depthStencil.stencil = rendering_info.stencil_attachment.clear_color.stencil;

					begin_rendering_info.pStencilAttachment = &stencil_attachment;
				}

				handle.beginRenderingKHR(begin_rendering_info);
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