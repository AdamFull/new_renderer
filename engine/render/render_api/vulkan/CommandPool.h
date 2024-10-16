#pragma once

#include <engine/render/render_api/vulkan/CommandBuffer.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class CommandPool;
			class RenderDevice;

			class CommandPoolBuilder
			{
			public:
				std::string debug_name{};
				CommandBuffer::ResetMode reset_mode{};
				vk::CommandPoolCreateInfo create_info{};

				CommandPoolBuilder& SetDebugName(const std::string& name);
				CommandPoolBuilder& SetResetMode(CommandBuffer::ResetMode reset_mode);
				CommandPoolBuilder& SetQueueFamily(uint32_t family_index);

				CommandPool Build(RenderDevice& render_device) const;
				std::unique_ptr<CommandPool> BuildUnique(RenderDevice& render_device) const;
			};

			class CommandPool final : public VulkanResource<vk::CommandPool>
			{
			public:
				CommandPool(RenderDevice& render_device, const CommandPoolBuilder& builder);

				CommandPool(const CommandPool&) = delete;
				CommandPool(CommandPool&& other);
				~CommandPool();

				CommandPool& operator=(const CommandPool&) = delete;
				CommandPool& operator=(CommandPool&&) = delete;

				void FreeCommandBuffer(vk::CommandBuffer& command_buffer);
				vk::CommandBuffer AllocateCommandBuffer(vk::CommandBufferLevel level);

				void Reset();
				CommandBuffer& RequestCommandBuffer(vk::CommandBufferLevel level);

				RenderDevice& GetDevice();
				RenderDevice const& GetDevice() const;

				uint32_t GetQueueFamilyIndex() const;
				CommandBuffer::ResetMode GetResetMode() const;
			protected:
				void ResetCommandBuffers();
			private:
				RenderDevice& render_device;

				uint32_t queue_family_index{ 0u };
				std::vector<std::unique_ptr<CommandBuffer>> primary_command_buffers;
				uint32_t active_primary_command_buffer_count{ 0u };
				std::vector<std::unique_ptr<CommandBuffer>> secondary_command_buffers;
				uint32_t active_secondary_command_buffer_count{ 0u };
				CommandBuffer::ResetMode reset_mode;

				std::vector<vk::CommandBuffer> free_command_buffer_list{};
			};
		}
	}
}