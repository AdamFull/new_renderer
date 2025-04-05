#include "gfx_vk_command_pool.h"
#include "gfx_vk_debug.h"

#include "gfx_vk_device.h"
#include "gfx_vk_queue.h"
#include "gfx_vk_query_pool.h"
#include "gfx_vk_buffer.h"
#include "gfx_vk_texture.h"
#include "gfx_vk_pipeline.h"
#include "gfx_vk_root_signature.h"
#include "gfx_vk_descriptor_set.h"
#include "gfx_vk_resource_descriptor.h"

#include "../gfx_util.h"
#include "gfx_vk_util.h"

#if USE_NSIGHT_AFTERMATH
#include "../gfx_nsight_aftermath.h"
#endif

namespace edge::gfx::vulkan
{
	VulkanCommandList::VulkanCommandList(VulkanCommandPool& command_pool, vk::CommandBufferLevel level) :
		VulkanResource{ nullptr, command_pool.get_allocation_callbacks() },
		command_pool{ command_pool } {

		handle_ = command_pool.allocate_command_buffer(level);

		auto reset_mode = command_pool.get_reset_mode();
		reset(reset_mode);
	}

	VulkanCommandList::VulkanCommandList(VulkanCommandList&& other) :
		VulkanResource{ std::move(other) },
		command_pool{ other.command_pool } {
	}

	VulkanCommandList::~VulkanCommandList() {
		command_pool.free_command_buffer(handle_);
	}

	void VulkanCommandList::set_resource_name(std::string_view name) const noexcept {
		auto& device = command_pool.get_device();
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	void VulkanCommandList::reset(CommandPoolResetMode reset_mode) {
		if (reset_mode == CommandPoolResetMode::eResetIndividually) {
			handle_.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		}
	}

	void VulkanCommandList::begin() {
		vk::CommandBufferBeginInfo begin_info{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
		vk::Result result = handle_.begin(&begin_info);
	}

	void VulkanCommandList::end() {
		handle_.end();
	}

	void VulkanCommandList::set_viewport(float x, float y, float width, float height, float min_depth, float max_depth) const {
		vk::Viewport viewport{ x, y, width, height, min_depth, max_depth };
		handle_.setViewport(0u, 1u, &viewport);
	}

	void VulkanCommandList::set_scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const {
		vk::Rect2D scissor{ { static_cast<int32_t>(x), static_cast<int32_t>(y) }, { width, height } };
		handle_.setScissor(0u, 1u, &scissor);
	}

	void VulkanCommandList::bind_pipeline(IPipeline* pipeline) const {
		auto* vk_pipeline = utl::cast<VulkanPipeline*>(pipeline);
		auto bind_point = pipeline_type_to_vk(vk_pipeline->get_type());
		handle_.bindPipeline(bind_point, vk_pipeline->get_handle());
	}

	void VulkanCommandList::bind_descriptors(const BindDescriptorsInfo& bind_info) const {
		auto* vk_root_signature = utl::cast<VulkanRootSignature*>(bind_info.root_signature);

		uint32_t first_descriptor_set{ ~0u };
		uint32_t descriptor_set_count{ 0u };
		std::array<vk::DescriptorSet, k_max_descriptor_count> descriptor_sets{};
		for (int32_t i = 0; i < bind_info.resource_descriptors.size(); ++i) {
			auto* resource_descriptor = bind_info.resource_descriptors[i];
			if (resource_descriptor) {
				auto* vk_resource_descriptor = utl::cast<VulkanResourceDescriptor*>(resource_descriptor);
				auto first_set = vk_resource_descriptor->get_first_set();
				auto set_count = vk_resource_descriptor->get_set_count();
				auto& sets = vk_resource_descriptor->get_descriptor_sets();

				// TODO: check that descriptor ranges is not overlapping
				for (int32_t j = first_set; j < first_set + set_count; ++j) {
					descriptor_sets[j] = sets[j];
					first_descriptor_set = std::min(first_descriptor_set, (uint32_t)j);
					descriptor_set_count++;
				}
			}
		}

		auto& pipeline_layout = vk_root_signature->get_pipeline_layout();
		handle_.bindDescriptorSets(
			pipeline_type_to_vk(bind_info.pipeline_type),
			pipeline_layout.get_handle(),
			first_descriptor_set,
			descriptor_set_count,
			descriptor_sets.data() + first_descriptor_set,
			0u, nullptr);
	}

	void VulkanCommandList::push_constants(IShader* shader, IRootSignature* root_signature, std::span<const char> constant_data) const {
		auto* vk_root_signature = utl::cast<VulkanRootSignature*>(root_signature);
		auto& pipeline_layout = vk_root_signature->get_pipeline_layout();

		auto& reflection = shader->get_reflection();
		handle_.pushConstants(pipeline_layout.get_handle(), shader_stage_flags_to_vk(reflection.constants.stages), 0u, constant_data.size(), constant_data.data());
	}

	void VulkanCommandList::bind_index_buffer(IBuffer* index_buffer, IndexType index_type, uint64_t offset) const {
		vk::IndexType vk_index_type{ vk::IndexType::eUint32 };

		switch (index_type) {
		case gfx::IndexType::eUint16: vk_index_type = vk::IndexType::eUint16; break;
		case gfx::IndexType::eUint32: vk_index_type = vk::IndexType::eUint32; break;
		default:
			break;
		}

		auto* buffer = utl::cast<VulkanBuffer*>(index_buffer);
		handle_.bindIndexBuffer(buffer->get_handle(), offset, vk_index_type);
	}

	void VulkanCommandList::bind_vertex_buffers(std::span<IBuffer*> vertex_buffers, std::span<const uint32_t> strides, std::span<const uint64_t> offsets) const {
		std::array<vk::Buffer, 16ull> vk_vertex_buffers{};

		for (int32_t i = 0; i < vertex_buffers.size(); ++i) {
			auto* buffer = utl::cast<VulkanBuffer*>(vertex_buffers[i]);
			vk_vertex_buffers[i] = buffer->get_handle();
		}

		handle_.bindVertexBuffers(0u, vertex_buffers.size(), vk_vertex_buffers.data(), offsets.data());
	}

	void VulkanCommandList::draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) const {
		handle_.draw(vertex_count, instance_count, first_vertex, first_instance);
	}

	void VulkanCommandList::draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const {
		handle_.drawIndexed(index_count, instance_count, first_index, first_vertex, first_instance);
	}

	void VulkanCommandList::draw_indirect(const IndirectDrawCommand& draw_command) const {
		auto* argument_buffer = utl::cast<VulkanBuffer*>(draw_command.argument_buffer);
		auto* count_buffer = utl::cast<VulkanBuffer*>(draw_command.count_buffer);

		if (draw_command.indexed_draw) {
			if (count_buffer) {
				handle_.drawIndexedIndirectCount(argument_buffer->get_handle(), draw_command.argument_offset,
					count_buffer->get_handle(), draw_command.count_offset,
					draw_command.draw_count, draw_command.command_stride);
			}
			else {
				handle_.drawIndexedIndirect(argument_buffer->get_handle(), draw_command.argument_offset,
					draw_command.draw_count, draw_command.command_stride);
			}
		}
		else {
			if (count_buffer) {
				handle_.drawIndirectCount(argument_buffer->get_handle(), draw_command.argument_offset,
					count_buffer->get_handle(), draw_command.count_offset,
					draw_command.draw_count, draw_command.command_stride);
			}
			else {
				handle_.drawIndirect(argument_buffer->get_handle(), draw_command.argument_offset,
					draw_command.draw_count, draw_command.command_stride);
			}
		}
	}

	//void VulkanCommandList::write_timestamp(IQueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) {
	//	auto* pool = utl::cast<VulkanQueryPool*>(query_pool);
	//	handle_.writeTimestamp2KHR(stage_flags_to_vk(stage_flag), pool->get_handle(), pool_index);
	//}

	void VulkanCommandList::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const {
		handle_.dispatch(group_x, group_y, group_z);
	}

	void VulkanCommandList::dispatch_indirect(IBuffer* argument_buffer, uint64_t argument_offset) const {
		auto* vk_buffer = utl::cast<VulkanBuffer*>(argument_buffer);
		handle_.dispatchIndirect(vk_buffer->get_handle(), argument_offset);
	}

	void VulkanCommandList::copy_buffer(const CopyBufferInfo& copy_info) const {
		std::array<vk::BufferCopy2KHR, 64ull> regions{};
		vk::CopyBufferInfo2KHR vk_copy_info{};
		vk_copy_info.pRegions = regions.data();

		auto* src_buffer = utl::cast<VulkanBuffer*>(copy_info.src_buffer);
		vk_copy_info.srcBuffer = src_buffer->get_handle();
		auto* dst_buffer = utl::cast<VulkanBuffer*>(copy_info.dst_buffer);
		vk_copy_info.dstBuffer = dst_buffer->get_handle();

		for (int32_t i = 0; i < copy_info.regions.size(); ++i) {
			auto& region = copy_info.regions[i];
			auto& vk_region = regions[vk_copy_info.regionCount++];
			vk_region.srcOffset = region.src_offset;
			vk_region.dstOffset = region.dst_offset;
			vk_region.size = region.size;
		}

		handle_.copyBuffer2KHR(vk_copy_info);
	}

	void VulkanCommandList::copy_texture(const CopyTextureInfo& copy_info) const {
		std::array<vk::BufferImageCopy2KHR, 64ull> regions{};
		vk::CopyBufferToImageInfo2KHR vk_copy_info{};
		vk_copy_info.pRegions = regions.data();

		auto* src_buffer = utl::cast<VulkanBuffer*>(copy_info.src_buffer);
		vk_copy_info.srcBuffer = src_buffer->get_handle();

		auto* dst_texture = utl::cast<VulkanTexture*>(copy_info.dst_texture);
		auto& dst_image = dst_texture->get_image();
		auto& subresource = dst_texture->get_subresource();
		vk_copy_info.dstImage = dst_image.get_handle();

		// TODO: As i think it will be transfer dst in any case
		vk_copy_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;

		for (int32_t i = 0; i < copy_info.copy_regions.size(); ++i) {
			auto& region = copy_info.copy_regions[i];
			auto& vk_region = regions[vk_copy_info.regionCount++];

			vk_region.bufferOffset = region.buffer_offset;
			vk_region.imageSubresource.aspectMask = subresource.get_aspect_mask();
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

		handle_.copyBufferToImage2KHR(vk_copy_info);
	}

	void VulkanCommandList::resource_barrier(const ResourceBarrierInfo& barrier) const {
		std::vector<vk::MemoryBarrier2KHR> memory_barriers{};
		if (!barrier.memory_barriers.empty())
			memory_barriers.resize(barrier.memory_barriers.size());

		std::vector<vk::BufferMemoryBarrier2KHR> buffer_barriers{};
		if (!barrier.buffer_barriers.empty())
			buffer_barriers.resize(barrier.buffer_barriers.size());

		std::vector<vk::ImageMemoryBarrier2KHR> image_barriers{};
		if (!barrier.texture_barriers.empty())
			image_barriers.resize(barrier.texture_barriers.size());

		vk::DependencyInfoKHR vk_barrier{};
		vk_barrier.pMemoryBarriers = memory_barriers.data();
		vk_barrier.pBufferMemoryBarriers = buffer_barriers.data();
		vk_barrier.pImageMemoryBarriers = image_barriers.data();

		// Parse memory barriers
		for (int32_t i = 0; i < barrier.memory_barriers.size(); ++i) {
			auto& memory_barrier = barrier.memory_barriers[i];

			auto& vk_memory_barrier = memory_barriers[vk_barrier.memoryBarrierCount++];
			vk_memory_barrier.srcAccessMask = resource_state_to_vk_access_flags(memory_barrier.src_state);
			vk_memory_barrier.srcStageMask = resource_state_to_vk_stage_flags(memory_barrier.src_state);
			vk_memory_barrier.dstAccessMask = resource_state_to_vk_access_flags(memory_barrier.dst_state);
			vk_memory_barrier.dstStageMask = resource_state_to_vk_stage_flags(memory_barrier.dst_state);
		}

		// Parse buffer barriers
		for (int32_t i = 0; i < barrier.buffer_barriers.size(); ++i) {
			auto& buffer_barrier = barrier.buffer_barriers[i];
			auto* buffer = utl::cast<VulkanBuffer*>(buffer_barrier.buffer);

			auto& vk_buffer_barrier = buffer_barriers[vk_barrier.bufferMemoryBarrierCount++];
			vk_buffer_barrier.buffer = buffer->get_handle();

			if (buffer_barrier.range.offset != 0ull && buffer_barrier.range.size != 0ull) {
				vk_buffer_barrier.offset = buffer_barrier.range.offset;
				vk_buffer_barrier.size = buffer_barrier.range.size;
			}
			else {
				vk_buffer_barrier.size = buffer->get_size();
			}

			vk_buffer_barrier.srcAccessMask = resource_state_to_vk_access_flags(buffer_barrier.src_state);
			vk_buffer_barrier.srcStageMask = resource_state_to_vk_stage_flags(buffer_barrier.src_state);
			vk_buffer_barrier.dstAccessMask = resource_state_to_vk_access_flags(buffer_barrier.dst_state);
			vk_buffer_barrier.dstStageMask = resource_state_to_vk_stage_flags(buffer_barrier.dst_state);

			// TODO: NOT IMPLEMENTED TRANSFER OWNERSHIP
			//if (buffer_barrier.src_state.queue && buffer_barrier.dst_state.queue)
			//{
			//	const auto* src_queue = utl::cast<const VulkanQueue*>(buffer_barrier.src_state.queue);
			//	const auto* dst_queue = utl::cast<const VulkanQueue*>(buffer_barrier.dst_state.queue);
			//	
			//	vk_buffer_barrier.srcQueueFamilyIndex = src_queue->get_family_index();
			//	vk_buffer_barrier.dstQueueFamilyIndex = dst_queue->get_family_index();
			//}
		}

		// Parse image barriers
		for (int32_t i = 0; i < barrier.texture_barriers.size(); ++i) {
			auto& texture_barrier = barrier.texture_barriers[i];
			auto* texture = utl::cast<VulkanTexture*>(texture_barrier.texture);

			auto& vk_image_barrier = image_barriers[vk_barrier.imageMemoryBarrierCount++];

			auto& image = texture->get_image();
			vk_image_barrier.image = image.get_handle();

			auto& subresource = texture->get_subresource(texture_barrier.subresource_index);
			vk_image_barrier.subresourceRange = subresource.get_subresource_range();

			vk_image_barrier.oldLayout = resource_state_to_vk_image_layout(texture_barrier.src_state);
			//assert(vk_image_barrier.oldLayout == subresource.GetImageLayout() && "Old layout is not correct."); 
			vk_image_barrier.srcAccessMask = resource_state_to_vk_access_flags(texture_barrier.src_state);
			vk_image_barrier.srcStageMask = resource_state_to_vk_stage_flags(texture_barrier.src_state);

			vk_image_barrier.newLayout = resource_state_to_vk_image_layout(texture_barrier.dst_state);
			vk_image_barrier.dstAccessMask = resource_state_to_vk_access_flags(texture_barrier.dst_state);
			vk_image_barrier.dstStageMask = resource_state_to_vk_stage_flags(texture_barrier.dst_state);

			// TODO: NOT IMPLEMENTED TRANSFER OWNERSHIP
			//if (src_state.queue && dst_state.queue)
			//{
			//	const auto* src_queue = utl::cast<const VulkanQueue*>(src_state.queue);
			//	const auto* dst_queue = utl::cast<const VulkanQueue*>(dst_state.queue);
			//
			//	vk_image_barrier.srcQueueFamilyIndex = src_queue->get_family_index();
			//	vk_image_barrier.dstQueueFamilyIndex = dst_queue->get_family_index();
			//}
		}

		handle_.pipelineBarrier2KHR(vk_barrier);
	}

	void VulkanCommandList::reset_query(IQueryPool* query_pool, uint32_t first_query, uint32_t query_count) const {
		auto* vk_query_pool = utl::cast<VulkanQueryPool*>(query_pool);
		handle_.resetQueryPool(vk_query_pool->get_handle(), first_query, query_count);
	}

	void VulkanCommandList::begin_query(IQueryPool* query_pool, uint32_t query) const {
		auto* vk_query_pool = utl::cast<VulkanQueryPool*>(query_pool);

		switch (vk_query_pool->get_query_type()) {
		case vk::QueryType::eTimestamp: {
			handle_.writeTimestamp2KHR(vk::PipelineStageFlagBits2KHR::eTopOfPipe, vk_query_pool->get_handle(), query);
		} break;
		case vk::QueryType::eOcclusion:
		case vk::QueryType::ePipelineStatistics: {
			handle_.beginQuery(vk_query_pool->get_handle(), query, {});
		};
		default: {
			assert(false && "UNSUPPORTED QUERY TYPE");
		} break;
		}
	}

	void VulkanCommandList::end_query(IQueryPool* query_pool, uint32_t query) const {
		auto* vk_query_pool = utl::cast<VulkanQueryPool*>(query_pool);

		switch (vk_query_pool->get_query_type()) {
		case vk::QueryType::eTimestamp: {
			handle_.writeTimestamp2KHR(vk::PipelineStageFlagBits2KHR::eBottomOfPipe, vk_query_pool->get_handle(), query);
		} break;
		case vk::QueryType::eOcclusion:
		case vk::QueryType::ePipelineStatistics: {
			handle_.endQuery(vk_query_pool->get_handle(), query);
		};
		default: {
			assert(false && "UNSUPPORTED QUERY TYPE");
		} break;
		}
	}

	void VulkanCommandList::begin_rendering(const RenderingInfo& rendering_info) const {
		vk::RenderingInfoKHR begin_rendering_info{};
		begin_rendering_info.renderArea.offset.x = rendering_info.offset_x;
		begin_rendering_info.renderArea.offset.y = rendering_info.offset_y;
		begin_rendering_info.renderArea.extent.width = rendering_info.width;
		begin_rendering_info.renderArea.extent.height = rendering_info.height;

		begin_rendering_info.layerCount = rendering_info.layer_count;

		std::array<vk::RenderingAttachmentInfo, 8ull> color_attachments{};
		begin_rendering_info.pColorAttachments = color_attachments.data();

		for (int32_t i = 0; i < rendering_info.attachment_count; ++i) {
			auto& attachment = rendering_info.attachments[i];
			auto& vk_attachment = color_attachments[begin_rendering_info.colorAttachmentCount++];

			assert(attachment.texture);
			auto* texture = utl::cast<VulkanTexture*>(attachment.texture);
			auto& subresource = texture->get_subresource();

			vk_attachment.imageView = subresource.get_handle();
			vk_attachment.imageLayout = resource_state_to_vk_image_layout(attachment.texture_state);
			vk_attachment.loadOp = load_op_to_vk(attachment.load_op);
			vk_attachment.storeOp = store_op_to_vk(attachment.store_op);

			vk_attachment.clearValue.color = attachment.clear_color.color;
		}

		vk::RenderingAttachmentInfo depth_attachment{};
		if (rendering_info.depth_attachment.texture) {
			auto* texture = utl::cast<VulkanTexture*>(rendering_info.depth_attachment.texture);
			auto& subresource = texture->get_subresource();

			depth_attachment.imageView = subresource.get_handle();
			depth_attachment.imageLayout = resource_state_to_vk_image_layout(rendering_info.depth_attachment.texture_state);
			depth_attachment.loadOp = load_op_to_vk(rendering_info.depth_attachment.load_op);
			depth_attachment.storeOp = store_op_to_vk(rendering_info.depth_attachment.store_op);

			depth_attachment.clearValue.depthStencil.depth = rendering_info.depth_attachment.clear_color.depth;
			depth_attachment.clearValue.depthStencil.stencil = rendering_info.depth_attachment.clear_color.stencil;

			begin_rendering_info.pDepthAttachment = &depth_attachment;
		}

		vk::RenderingAttachmentInfo stencil_attachment{};
		if (rendering_info.stencil_attachment.texture) {
			auto* texture = utl::cast<VulkanTexture*>(rendering_info.stencil_attachment.texture);
			auto& subresource = texture->get_subresource();

			stencil_attachment.imageView = subresource.get_handle();
			stencil_attachment.imageLayout = resource_state_to_vk_image_layout(rendering_info.stencil_attachment.texture_state);
			stencil_attachment.loadOp = load_op_to_vk(rendering_info.stencil_attachment.load_op);
			stencil_attachment.storeOp = store_op_to_vk(rendering_info.stencil_attachment.store_op);

			stencil_attachment.clearValue.depthStencil.depth = rendering_info.stencil_attachment.clear_color.depth;
			stencil_attachment.clearValue.depthStencil.stencil = rendering_info.stencil_attachment.clear_color.stencil;

			begin_rendering_info.pStencilAttachment = &stencil_attachment;
		}

		handle_.beginRenderingKHR(begin_rendering_info);
	}

	void VulkanCommandList::end_rendering() const {
		handle_.endRenderingKHR();
	}

	void VulkanCommandList::begin_marker(std::string_view name, uint32_t color) const {
		auto& device = command_pool.get_device();
		auto& debug_utils = device.get_debug_utils();
		debug_utils.begin_label(handle_, name, color);

#if USE_NSIGHT_AFTERMATH
		NsightAftermath::try_set_marker(this, name.data());
#endif
	}

	void VulkanCommandList::insert_marker(std::string_view name, uint32_t color) const {
		auto& device = command_pool.get_device();
		auto& debug_utils = device.get_debug_utils();
		debug_utils.insert_label(handle_, name, color);

#if USE_NSIGHT_AFTERMATH
		NsightAftermath::try_set_marker(this, name.data());
#endif
	}

	void VulkanCommandList::end_marker() const {
		auto& device = command_pool.get_device();
		auto& debug_utils = device.get_debug_utils();
		debug_utils.end_label(handle_);
	}
}