#pragma once

#include <VkResource.h>
#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class VulkanCommandPool;

		class VulkanCommandList final : public CommandList, public VulkanResource<vk::CommandBuffer>
		{
		public:
			enum class ResetMode
			{
				ResetPool,
				ResetIndividually,
				AlwaysAllocate,
			};

			VulkanCommandList(VulkanCommandPool& command_pool, vk::CommandBufferLevel level);
			VulkanCommandList(VulkanCommandList&& other);
			~VulkanCommandList();

			VulkanCommandList(const VulkanCommandList&) = delete;
			VulkanCommandList& operator=(const VulkanCommandList&) = delete;
			VulkanCommandList& operator=(VulkanCommandList&&) = delete;

			void Reset(CommandPoolResetMode reset_mode) override;

			void Begin() override;
			void End() override;

			// Dynamic states
			void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) override;
			void SerScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

			void BindPipeline(Pipeline* pipeline) override;
			void BindDescriptorSet() override;
			void BindDescriptorSets() override;
			void PushConstants(RootSignature* root_signature, std::span<const char> constant_data) override;

			void BindIndexBuffer(Buffer* index_buffer, IndexType index_type, uint64_t offset) override;
			void BindVertexBuffers(uint32_t buffer_count, Buffer** vertex_buffers, const uint32_t* strides, const uint64_t* offsets) override;

			void Draw(uint32_t vertex_count, uint32_t first_vertex, uint32_t first_instance, uint32_t instance_count) override;
			void DrawIndexed(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;
			void DrawIndirect() override;
			void DrawIndirectIndexed() override;
			void DrawIndirectIndexedCount() override;

			void WriteTimestamp(QueryPool* query_pool, uint32_t pool_index, StageFlag stage_flag) override;

			void Dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
			void DispatchIndirect() override;

			void PipelineBarrier() override;

			void BeginQuery(QueryPool* query_pool) override;
			void EndQuery(QueryPool* query_pool) override;

			void BeginRendering() override;
			void EndRendering() override;

			void BeginMarker(const char* name, uint32_t color) override;
			void InsertMarker(const char* name, uint32_t color) override;
			void EndMarker() override;
		private:
			VulkanCommandPool& command_pool;
		};
	}
}