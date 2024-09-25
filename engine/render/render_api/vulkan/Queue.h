#pragma once

#include <engine/render/render_api/vulkan/VkResource.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Device;
			class CommandBuffer;

			class Queue : public VulkanResource<vk::Queue>
			{
			public:
				Queue(Device& device, uint32_t family_index, vk::QueueFamilyProperties properties, vk::Bool32 can_present, uint32_t index);

				Queue(const Queue&) = default;

				Queue(Queue&& other);

				Queue& operator=(const Queue&) = delete;

				Queue& operator=(Queue&&) = delete;

				const Device& GetDevice() const;

				uint32_t GetFamilyIndex() const;

				uint32_t GetIndex() const;

				const vk::QueueFamilyProperties& GetProperties() const;

				vk::Bool32 SupportPresent() const;

				void Submit(const vk::CommandBuffer& command_buffer, vk::Fence fence) const;

				vk::Result Present(const vk::PresentInfoKHR& present_infos) const;

			private:
				Device& device;

				uint32_t family_index{ 0 };

				uint32_t index{ 0 };

				vk::Bool32 can_present{ false };

				vk::QueueFamilyProperties properties{};
			};
		}
	}
}