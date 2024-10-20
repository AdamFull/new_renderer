#include <base/Queue.h>
#include <base/Device.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			Queue::Queue(Device& device, uint32_t family_index, vk::QueueFamilyProperties properties, vk::Bool32 can_present, uint32_t index) :
				VulkanResource{ nullptr },
				device{ device },
				family_index{ family_index },
				index{ index },
				can_present{ can_present },
				properties{ properties }
			{
				handle = Handle(device).getQueue(family_index, index);
			}

			Queue::Queue(Queue&& other) :
				VulkanResource{ std::move(other) },
				device(other.device),
				family_index(std::exchange(other.family_index, {})),
				index(std::exchange(other.index, 0)),
				can_present(std::exchange(other.can_present, false)),
				properties(std::exchange(other.properties, {}))
			{
			}

			const Device& Queue::GetDevice() const
			{
				return device;
			}

			uint32_t Queue::GetFamilyIndex() const
			{
				return family_index;
			}

			uint32_t Queue::GetIndex() const
			{
				return index;
			}

			const vk::QueueFamilyProperties& Queue::GetProperties() const
			{
				return properties;
			}

			vk::Bool32 Queue::SupportPresent() const
			{
				return can_present;
			}

			void Queue::Submit(const vk::CommandBuffer& command_buffer, vk::Fence fence) const
			{
				vk::SubmitInfo submit_info({}, {}, command_buffer);
				handle.submit(submit_info, fence);
			}

			void Queue::Submit(const vk::SubmitInfo& submit_info, vk::Fence fence) const
			{
				handle.submit(submit_info, fence);
			}

			void Queue::Submit(const vk::SubmitInfo2& submit_info, vk::Fence fence) const
			{
				handle.submit2(submit_info, fence);
			}

			vk::Result Queue::Present(const vk::PresentInfoKHR& present_info) const
			{
				if (!can_present)
				{
					return vk::Result::eErrorIncompatibleDisplayKHR;
				}

				return handle.presentKHR(present_info);
			}
		}
	}
}