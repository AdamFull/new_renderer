#pragma once

#include <engine/render/render_api/vulkan/VkResource.h>
#include <engine/render/render_api/vulkan/VkResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;
			class CommandPool;

			class CommandBuffer final : public VulkanResource<vk::CommandBuffer>
			{
			public:
				enum class ResetMode
				{
					ResetPool,
					ResetIndividually,
					AlwaysAllocate,
				};

				CommandBuffer(CommandPool& command_pool, vk::CommandBufferLevel level);
				CommandBuffer(CommandBuffer&& other);
				~CommandBuffer();

				CommandBuffer(const CommandBuffer&) = delete;
				CommandBuffer& operator=(const CommandBuffer&) = delete;
				CommandBuffer& operator=(CommandBuffer&&) = delete;

				vk::Result Begin(vk::CommandBufferUsageFlags flags, CommandBuffer* primary = nullptr);
				vk::Result Begin(vk::CommandBufferUsageFlags flags, vk::RenderPass* render_pass, vk::Framebuffer* framebuffer, uint32_t subpass);

				void End();

				void Reset(ResetMode reset_mode);

				void BindPipeline(vk::PipelineBindPoint bind_point, vk::Pipeline& pipeline);
				void PushConstants(const vk::PushConstantsInfoKHR& push_info);

				void BindVertexBuffers(uint32_t first_binding, const std::vector<vk::Buffer>& buffers, const std::vector<uint64_t>& offsets);

				void ResetTimestampQuery(FVkTimestampQuery& timestamp_query);
				void WriteTimestampQuery(vk::PipelineStageFlagBits pipeline_state, FVkTimestampQuery& timestamp_query);

				void BeginDebugLabel(const std::string& label, u32 color);
				void InsertDebugLabel(const std::string& label, u32 color);
				void EndDebugLabel();

			private:
				CommandPool& command_pool;
			};
		}
	}
}