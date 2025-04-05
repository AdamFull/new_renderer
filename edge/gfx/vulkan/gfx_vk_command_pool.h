#pragma once

#include "gfx_vk_command_list.h"

namespace edge::gfx::vulkan
{
	class Device;
	class DebugUtils;
	class Queue;

	class CommandPool final : public ICommandPool, public Resource<vk::CommandPool> {
	public:
		CommandPool(const Queue& queue, CommandPoolResetMode reset_mode);

		CommandPool(const CommandPool&) = delete;
		CommandPool(CommandPool&& other);
		~CommandPool();

		CommandPool& operator=(const ICommandPool&) = delete;
		CommandPool& operator=(ICommandPool&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		[[nodiscard]] auto allocate(bool secondary) -> CommandList* override;
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
		std::vector<std::unique_ptr<CommandList>> primary_command_buffers;
		uint32_t active_primary_command_buffer_count{ 0u };
		std::vector<std::unique_ptr<CommandList>> secondary_command_buffers;
		uint32_t active_secondary_command_buffer_count{ 0u };
		CommandPoolResetMode reset_mode;

		std::vector<vk::CommandBuffer> free_command_buffer_list{};
	};
}