#pragma once

#include "VkResource.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class VulkanCommandPool;

			class VulkanCommandList final : public ICommandList, public VulkanResource<vk::CommandBuffer> {
			public:
				VulkanCommandList(VulkanCommandPool& command_pool, vk::CommandBufferLevel level);
				VulkanCommandList(VulkanCommandList&& other);
				~VulkanCommandList();

				VulkanCommandList(const VulkanCommandList&) = delete;
				VulkanCommandList& operator=(const VulkanCommandList&) = delete;
				VulkanCommandList& operator=(VulkanCommandList&&) = delete;

				void set_resource_name(std::string_view name) const noexcept override;

				void reset(CommandPoolResetMode reset_mode) override;

				void begin() override;
				void end() override;

				// Dynamic states
				void set_viewport(float x, float y, float width, float height, float min_depth, float max_depth) const override;
				void set_scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const override;

				void bind_pipeline(IPipeline* pipeline) const override;
				void bind_descriptors(const BindDescriptorsInfo& bind_info) const override;
				void push_constants(IShader* shader, IRootSignature* root_signature, std::span<const char> constant_data) const override;

				void bind_index_buffer(IBuffer* index_buffer, IndexType index_type, uint64_t offset) const override;
				void bind_vertex_buffers(std::span<IBuffer*> vertex_buffers, std::span<const uint32_t> strides, std::span<const uint64_t> offsets) const override;

				void draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) const override;
				void draw_indexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const override;
				void draw_indirect(const IndirectDrawCommand& draw_command) const override;

				//void write_timestamp(IQueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) override;

				void dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const override;
				void dispatch_indirect(IBuffer* argument_buffer, uint64_t argument_offset = 0ull) const override;

				void copy_buffer(const CopyBufferInfo& copy_info) const override;
				void copy_texture(const CopyTextureInfo& copy_info) const override;

				void resource_barrier(const ResourceBarrierInfo& barrier) const override;

				void reset_query(IQueryPool* query_pool, uint32_t first_query, uint32_t query_count) const override;
				void begin_query(IQueryPool* query_pool, uint32_t query) const override;
				void end_query(IQueryPool* query_pool, uint32_t query) const override;

				void begin_rendering(const RenderingInfo& rendering_info) const override;
				void end_rendering() const override;

				void begin_marker(std::string_view name, uint32_t color) const override;
				void insert_marker(std::string_view name, uint32_t color) const override;
				void end_marker() const override;
			private:
				VulkanCommandPool& command_pool;
			};
		}
	}
}