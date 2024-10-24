#include <VkCommandPool.h>
#include <VkDevice.h>
#include <VkDebug.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanCommandPool::VulkanCommandPool(const VulkanQueue& queue, DebugUtils& debug_utils, CommandPoolResetMode reset_mode) :
				VulkanResource{ nullptr, queue.GetAllocationCallbacks() },
				device{ queue.GetDevice() },
				debug_utils{ debug_utils },
				queue_family_index{ queue.GetFamilyIndex() },
				reset_mode{ reset_mode }
			{
				vk::CommandPoolCreateInfo create_info{};
				create_info.queueFamilyIndex = queue_family_index;

				switch (reset_mode)
				{
				case CommandPoolResetMode::eResetIndividually:
				case CommandPoolResetMode::eAlwaysAllocate:
					create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
					break;
				case CommandPoolResetMode::eResetPool:
				default:
					create_info.flags = vk::CommandPoolCreateFlagBits::eTransient;
					break;
				}

				vk::Result result = Handle(device).createCommandPool(&create_info, allocation_callbacks, &handle);
			}

			VulkanCommandPool::VulkanCommandPool(VulkanCommandPool&& other) :
				VulkanResource{ std::move(other) },
				device{ device },
				debug_utils{ other.debug_utils },
				queue_family_index{ other.queue_family_index },
				primary_command_buffers{ std::move(other.primary_command_buffers) },
				active_primary_command_buffer_count(std::exchange(other.active_primary_command_buffer_count, {})),
				secondary_command_buffers{ std::move(other.secondary_command_buffers) },
				active_secondary_command_buffer_count(std::exchange(other.active_secondary_command_buffer_count, {})),
				reset_mode{ std::exchange(other.reset_mode, {}) }
			{

			}

			VulkanCommandPool::~VulkanCommandPool()
			{
				primary_command_buffers.clear();
				secondary_command_buffers.clear();

				auto& vkdevice = device.GetHandle();

				vkdevice.freeCommandBuffers(handle, free_command_buffer_list.size(), free_command_buffer_list.data());
				free_command_buffer_list.clear();

				vkdevice.destroyCommandPool(handle, allocation_callbacks);
			}

			VulkanCommandList* VulkanCommandPool::Allocate(bool secondary)
			{
				std::vector<std::unique_ptr<VulkanCommandList>>& buffers =
					secondary ? secondary_command_buffers : primary_command_buffers;

				uint32_t& active_count = secondary ? active_secondary_command_buffer_count : active_primary_command_buffer_count;

				// Check if an existing buffer can be reused
				if (active_count < buffers.size())
				{
					return buffers[active_count++].get();
				}

				// Create a new command buffer if none are available
				buffers.emplace_back(std::make_unique<VulkanCommandList>(*this, secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary));
				return buffers.back().get();
			}


			void VulkanCommandPool::Reset()
			{
				switch (reset_mode)
				{
				case CommandPoolResetMode::eResetIndividually:
					ResetCommandBuffers();
					break;

				case CommandPoolResetMode::eResetPool:
					Handle(device).resetCommandPool(handle);
					ResetCommandBuffers();
					break;

				case CommandPoolResetMode::eAlwaysAllocate:
					primary_command_buffers.clear();
					active_primary_command_buffer_count = 0;
					secondary_command_buffers.clear();
					active_secondary_command_buffer_count = 0;
					break;

				default:
					throw std::runtime_error("Unknown reset mode for command pools");
				}

				FreeCommandBuffers();
			}

			vk::CommandBuffer VulkanCommandPool::AllocateCommandBuffer(vk::CommandBufferLevel level)
			{
				vk::CommandBufferAllocateInfo allocate_info(handle, level, 1);

				vk::CommandBuffer command_buffer{};
				vk::Result result = Handle(device).allocateCommandBuffers(&allocate_info, &command_buffer);

				return command_buffer;
			}

			void VulkanCommandPool::FreeCommandBuffer(vk::CommandBuffer& command_buffer)
			{
				free_command_buffer_list.emplace_back(command_buffer);
			}

			CommandPoolResetMode VulkanCommandPool::GetResetMode() const
			{
				return reset_mode;
			}

			DebugUtils& VulkanCommandPool::GetDebugUtils()
			{
				return debug_utils;
			}

			DebugUtils const& VulkanCommandPool::GetDebugUtils() const
			{
				return debug_utils;
			}

			void VulkanCommandPool::FreeCommandBuffers()
			{
				if (!free_command_buffer_list.empty())
				{
					Handle(device).freeCommandBuffers(handle, free_command_buffer_list.size(), free_command_buffer_list.data());
					free_command_buffer_list.clear();
				}
			}

			void VulkanCommandPool::ResetCommandBuffers()
			{
				for (auto& cmd_buf : primary_command_buffers)
				{
					cmd_buf->Reset(reset_mode);
				}
				active_primary_command_buffer_count = 0;

				for (auto& cmd_buf : secondary_command_buffers)
				{
					cmd_buf->Reset(reset_mode);
				}
				active_secondary_command_buffer_count = 0;
			}
		}
	}
}