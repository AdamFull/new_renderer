#include <base/CommandPool.h>
#include <RenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			CommandPoolBuilder& CommandPoolBuilder::SetDebugName(const std::string& name)
			{
				debug_name = name;
				return *this;
			}

			CommandPoolBuilder& CommandPoolBuilder::SetResetMode(CommandBuffer::ResetMode reset_mode)
			{
				this->reset_mode = reset_mode;

				switch (reset_mode)
				{
				case CommandBuffer::ResetMode::ResetIndividually:
				case CommandBuffer::ResetMode::AlwaysAllocate:
					create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
					break;
				case CommandBuffer::ResetMode::ResetPool:
				default:
					create_info.flags = vk::CommandPoolCreateFlagBits::eTransient;
					break;
				}

				return *this;
			}

			CommandPoolBuilder& CommandPoolBuilder::SetQueueFamily(uint32_t family_index)
			{
				create_info.queueFamilyIndex = family_index;
				return *this;
			}

			CommandPool CommandPoolBuilder::Build(RenderDevice& render_device) const
			{
				return CommandPool(render_device, *this);
			}

			std::unique_ptr<CommandPool> CommandPoolBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<CommandPool>(render_device, *this);
			}

			CommandPool::CommandPool(RenderDevice& render_device, const CommandPoolBuilder& builder) :
				VulkanResource{ nullptr },
				render_device{ render_device },
				queue_family_index{ builder.create_info.queueFamilyIndex },
				reset_mode{ builder.reset_mode }
			{
				handle = render_device.CreateCommandPool2(builder.create_info, builder.debug_name);
			}

			CommandPool::CommandPool(CommandPool&& other) :
				VulkanResource{ std::move(other) },
				render_device{ render_device },
				queue_family_index{ other.queue_family_index },
				primary_command_buffers{ std::move(other.primary_command_buffers) },
				active_primary_command_buffer_count(std::exchange(other.active_primary_command_buffer_count, {})),
				secondary_command_buffers{ std::move(other.secondary_command_buffers) },
				active_secondary_command_buffer_count(std::exchange(other.active_secondary_command_buffer_count, {})),
				reset_mode{ std::exchange(other.reset_mode, {}) }
			{

			}

			CommandPool::~CommandPool()
			{
				primary_command_buffers.clear();
				secondary_command_buffers.clear();

				render_device.FreeCommandBuffers(handle, free_command_buffer_list.data(), free_command_buffer_list.size());
				free_command_buffer_list.clear();

				render_device.Destroy(handle);
			}

			void CommandPool::FreeCommandBuffer(vk::CommandBuffer& command_buffer)
			{
				free_command_buffer_list.push_back(command_buffer);
			}

			vk::CommandBuffer CommandPool::AllocateCommandBuffer(vk::CommandBufferLevel level)
			{
				vk::CommandBufferAllocateInfo allocate_info(handle, level, 1);

				vk::CommandBuffer command_buffer{};
				render_device.AllocateCommandBuffers(allocate_info, &command_buffer);

				return command_buffer;
			}

			void CommandPool::Reset()
			{
				switch (reset_mode)
				{
				case CommandBuffer::ResetMode::ResetIndividually:
					ResetCommandBuffers();
					break;

				case CommandBuffer::ResetMode::ResetPool:
					render_device.ResetCommandPool(handle);
					ResetCommandBuffers();
					break;

				case CommandBuffer::ResetMode::AlwaysAllocate:
					primary_command_buffers.clear();
					active_primary_command_buffer_count = 0;
					secondary_command_buffers.clear();
					active_secondary_command_buffer_count = 0;
					break;

				default:
					throw std::runtime_error("Unknown reset mode for command pools");
				}

				if (!free_command_buffer_list.empty())
				{
					render_device.FreeCommandBuffers(handle, free_command_buffer_list.data(), free_command_buffer_list.size());
					free_command_buffer_list.clear();
				}
			}

			CommandBuffer& CommandPool::RequestCommandBuffer(vk::CommandBufferLevel level)
			{
				std::vector<std::unique_ptr<CommandBuffer>>& buffers =
					(level == vk::CommandBufferLevel::ePrimary) ? primary_command_buffers : secondary_command_buffers;

				uint32_t& active_count = (level == vk::CommandBufferLevel::ePrimary) ? active_primary_command_buffer_count : active_secondary_command_buffer_count;

				// Check if an existing buffer can be reused
				if (active_count < buffers.size())
				{
					return *buffers[active_count++];
				}

				// Create a new command buffer if none are available
				buffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));
				return *buffers.back();
			}

			RenderDevice& CommandPool::GetDevice()
			{
				return render_device;
			}

			RenderDevice const& CommandPool::GetDevice() const
			{
				return render_device;
			}

			uint32_t CommandPool::GetQueueFamilyIndex() const
			{
				return queue_family_index;
			}

			CommandBuffer::ResetMode CommandPool::GetResetMode() const
			{
				return reset_mode;
			}

			void CommandPool::ResetCommandBuffers()
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