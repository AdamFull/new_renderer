#pragma once

#include <VkResource.h>
#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		class VulkanQueue : public Queue, public VulkanResource<vk::Queue>
		{
		public:
			VulkanQueue(Device& device, uint32_t family_index, vk::QueueFamilyProperties properties, vk::Bool32 can_present, uint32_t index);
			VulkanQueue(VulkanQueue&& other);
			~VulkanQueue() = default;

			VulkanQueue(const VulkanQueue&) = default;
			VulkanQueue& operator=(const VulkanQueue&) = delete;
			VulkanQueue& operator=(VulkanQueue&&) = delete;

			const Device& GetDevice() const;
			Device& GetDevice();

			uint32_t GetFamilyIndex() const;
			uint32_t GetIndex() const;

			const vk::QueueFamilyProperties& GetProperties() const;

			vk::Bool32 SupportPresent() const;

			void Submit(const SubmitInfo& submit_info, Fence* fence) override;
			void Present(const PresentInfo& present_info) override;
			void WaitIdle() override;
		private:
			Device& device;

			uint32_t family_index{ 0 };

			uint32_t index{ 0 };

			vk::Bool32 can_present{ false };

			vk::QueueFamilyProperties properties{};
		};
	}
}