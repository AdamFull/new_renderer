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

			class VulkanCommandList final : public ICommandList, public VulkanResource<vk::CommandBuffer>
			{
			public:
				VulkanCommandList(VulkanCommandPool& command_pool, vk::CommandBufferLevel level);
				VulkanCommandList(VulkanCommandList&& other);
				~VulkanCommandList();

				VulkanCommandList(const VulkanCommandList&) = delete;
				VulkanCommandList& operator=(const VulkanCommandList&) = delete;
				VulkanCommandList& operator=(VulkanCommandList&&) = delete;

				void SetResourceName(std::string_view name) const noexcept override;

				void Reset(CommandPoolResetMode reset_mode) override;

				void Begin() override;
				void End() override;

				// Dynamic states
				void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) const override;
				void SerScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const override;

				void BindPipeline(IPipeline* pipeline) const override;
				void BindDescriptors(const BindDescriptorsInfo& bind_info) const override;
				void PushConstants(IShader* shader, IRootSignature* root_signature, std::span<const char> constant_data) const override;

				void BindIndexBuffer(IBuffer* index_buffer, IndexType index_type, uint64_t offset) const override;
				void BindVertexBuffers(std::span<IBuffer*> vertex_buffers, std::span<const uint32_t> strides, std::span<const uint64_t> offsets) const override;

				void Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) const override;
				void DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const override;
				void DrawIndirect(const IndirectDrawCommand& draw_command) const override;

				//void WriteTimestamp(IQueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) override;

				void Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) const override;
				void DispatchIndirect(IBuffer* argument_buffer, uint64_t argument_offset = 0ull) const override;

				void CopyBuffer(const CopyBufferInfo& copy_info) const override;
				void CopyTexture(const CopyTextureInfo& copy_info) const override;

				void ResourceBarrier(const ResourceBarrierInfo& barrier) const override;

				void ResetQuery(IQueryPool* query_pool, uint32_t first_query, uint32_t query_count) const override;
				void BeginQuery(IQueryPool* query_pool, uint32_t query) const override;
				void EndQuery(IQueryPool* query_pool, uint32_t query) const override;

				void BeginRendering(const RenderingInfo& rendering_info) const override;
				void EndRendering() const override;

				void BeginMarker(std::string_view name, uint32_t color) const override;
				void InsertMarker(std::string_view name, uint32_t color) const override;
				void EndMarker() const override;
			private:
				VulkanCommandPool& command_pool;
			};
		}
	}
}