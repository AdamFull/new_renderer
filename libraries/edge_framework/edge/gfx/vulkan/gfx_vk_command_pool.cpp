#include "gfx_vk_command_pool.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "../../util/Logger.h"

namespace edge::gfx::vulkan
{
	VulkanCommandPool::VulkanCommandPool(const VulkanQueue& queue, CommandPoolResetMode reset_mode) :
		VulkanResource{ nullptr, queue.get_allocation_callbacks() },
		device{ queue.get_family().get_device() },
		queue_family_index{ queue.get_family().get_family_index() },
		reset_mode{ reset_mode } {

		vk::CommandPoolCreateInfo create_info{};
		create_info.queueFamilyIndex = queue_family_index;

		switch (reset_mode) {
		case CommandPoolResetMode::eResetIndividually:
		case CommandPoolResetMode::eAlwaysAllocate:
			create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
			break;
		case CommandPoolResetMode::eResetPool:
		default:
			create_info.flags = vk::CommandPoolCreateFlagBits::eTransient;
			break;
		}

		vk::Result result = device.get_handle().createCommandPool(&create_info, allocation_callbacks, &handle_);
	}

	VulkanCommandPool::VulkanCommandPool(VulkanCommandPool&& other) :
		VulkanResource{ std::move(other) },
		device{ device },
		queue_family_index{ other.queue_family_index },
		primary_command_buffers{ std::move(other.primary_command_buffers) },
		active_primary_command_buffer_count(std::exchange(other.active_primary_command_buffer_count, {})),
		secondary_command_buffers{ std::move(other.secondary_command_buffers) },
		active_secondary_command_buffer_count(std::exchange(other.active_secondary_command_buffer_count, {})),
		reset_mode{ std::exchange(other.reset_mode, {}) }
	{

	}

	VulkanCommandPool::~VulkanCommandPool() {
		primary_command_buffers.clear();
		secondary_command_buffers.clear();

		auto& vkdevice = device.get_handle();

		if (!free_command_buffer_list.empty()) {
			vkdevice.freeCommandBuffers(handle_, free_command_buffer_list.size(), free_command_buffer_list.data());
			free_command_buffer_list.clear();
		}

		vkdevice.destroyCommandPool(handle_, allocation_callbacks);
	}

	void VulkanCommandPool::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	auto VulkanCommandPool::allocate(bool secondary) -> VulkanCommandList* {
		std::vector<std::unique_ptr<VulkanCommandList>>& buffers =
			secondary ? secondary_command_buffers : primary_command_buffers;

		uint32_t& active_count = secondary ? active_secondary_command_buffer_count : active_primary_command_buffer_count;

		// Check if an existing buffer can be reused
		if (active_count < buffers.size()) {
			return buffers[active_count++].get();
		}

		// Create a new command buffer if none are available
		buffers.emplace_back(std::make_unique<VulkanCommandList>(*this, secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary));
		return buffers.back().get();
	}


	void VulkanCommandPool::reset() noexcept {
		switch (reset_mode) {
		case CommandPoolResetMode::eResetIndividually:
			reset_command_buffers();
			break;
		case CommandPoolResetMode::eResetPool:
			device.get_handle().resetCommandPool(handle_);
			reset_command_buffers();
			break;
		case CommandPoolResetMode::eAlwaysAllocate:
			primary_command_buffers.clear();
			active_primary_command_buffer_count = 0;
			secondary_command_buffers.clear();
			active_secondary_command_buffer_count = 0;
			break;

		default:
			EDGE_LOGE("[VkCommandPool] Unknown reset mode for command pools");
		}

		free_command_buffers();
	}

	auto VulkanCommandPool::allocate_command_buffer(vk::CommandBufferLevel level) -> vk::CommandBuffer {
		vk::CommandBufferAllocateInfo allocate_info(handle_, level, 1);

		vk::CommandBuffer command_buffer{};
		vk::Result result = device.get_handle().allocateCommandBuffers(&allocate_info, &command_buffer);

		return command_buffer;
	}

	void VulkanCommandPool::free_command_buffer(vk::CommandBuffer& command_buffer) {
		free_command_buffer_list.emplace_back(command_buffer);
	}

	auto VulkanCommandPool::get_reset_mode() const noexcept -> CommandPoolResetMode {
		return reset_mode;
	}

	auto VulkanCommandPool::get_device() const noexcept -> Device const& {
		return device;
	}

	void VulkanCommandPool::free_command_buffers() {
		if (!free_command_buffer_list.empty()) {
			device.get_handle().freeCommandBuffers(handle_, free_command_buffer_list.size(), free_command_buffer_list.data());
			free_command_buffer_list.clear();
		}
	}

	void VulkanCommandPool::reset_command_buffers() {
		for (auto& cmd_buf : primary_command_buffers) {
			cmd_buf->reset(reset_mode);
		}
		active_primary_command_buffer_count = 0;

		for (auto& cmd_buf : secondary_command_buffers) {
			cmd_buf->reset(reset_mode);
		}
		active_secondary_command_buffer_count = 0;
	}
}