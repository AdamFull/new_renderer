#pragma once

#include <VkCommandList.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;
			class VulkanQueue;
			class DebugUtils;

			class VulkanCommandPool final : public CommandPool, public VulkanResource<vk::CommandPool>
			{
			public:
				VulkanCommandPool(const VulkanQueue& queue, DebugUtils& debug_utils, CommandPoolResetMode reset_mode);

				VulkanCommandPool(const VulkanCommandPool&) = delete;
				VulkanCommandPool(VulkanCommandPool&& other);
				~VulkanCommandPool();

				VulkanCommandPool& operator=(const CommandPool&) = delete;
				VulkanCommandPool& operator=(CommandPool&&) = delete;

				VulkanCommandList* Allocate(bool secondary) override;
				void Reset() override;

				vk::CommandBuffer AllocateCommandBuffer(vk::CommandBufferLevel level);
				void FreeCommandBuffer(vk::CommandBuffer& command_buffer);

				CommandPoolResetMode GetResetMode() const;

				DebugUtils& GetDebugUtils();
				DebugUtils const& GetDebugUtils() const;
			protected:
				void FreeCommandBuffers();
				void ResetCommandBuffers();
			private:
				const Device& device;
				DebugUtils& debug_utils;

				uint32_t queue_family_index{ 0u };
				std::vector<std::unique_ptr<VulkanCommandList>> primary_command_buffers;
				uint32_t active_primary_command_buffer_count{ 0u };
				std::vector<std::unique_ptr<VulkanCommandList>> secondary_command_buffers;
				uint32_t active_secondary_command_buffer_count{ 0u };
				CommandPoolResetMode reset_mode;

				std::vector<vk::CommandBuffer> free_command_buffer_list{};
			};
		}
	}
}