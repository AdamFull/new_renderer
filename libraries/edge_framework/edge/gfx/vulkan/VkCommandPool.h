#pragma once

#include "VkCommandList.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;
			class DebugUtils;
			class VulkanQueue;

			class VulkanCommandPool final : public ICommandPool, public VulkanResource<vk::CommandPool> {
			public:
				VulkanCommandPool(const VulkanQueue& queue, CommandPoolResetMode reset_mode);

				VulkanCommandPool(const VulkanCommandPool&) = delete;
				VulkanCommandPool(VulkanCommandPool&& other);
				~VulkanCommandPool();

				VulkanCommandPool& operator=(const ICommandPool&) = delete;
				VulkanCommandPool& operator=(ICommandPool&&) = delete;

				void set_resource_name(std::string_view name) const noexcept override;

				[[nodiscard]] auto allocate(bool secondary) -> VulkanCommandList* override;
				void reset() noexcept override;

				[[nodiscard]] auto allocate_command_buffer(vk::CommandBufferLevel level) -> vk::CommandBuffer;
				void free_command_buffer(vk::CommandBuffer& command_buffer);

				auto get_reset_mode() const noexcept -> CommandPoolResetMode;

				auto get_device() const noexcept -> Device const&;
			protected:
				void free_command_buffers();
				void reset_command_buffers();
			private:
				const Device& device;

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