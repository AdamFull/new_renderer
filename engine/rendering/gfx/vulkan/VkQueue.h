#pragma once

#include "VkResource.h"
#include <Base.h>

namespace engine
{
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

				void SetResourceName(const char* name) override;

				void Submit(const SubmitInfo& submit_info, Fence* fence) const override;
				void Present(const PresentInfo& present_info) const override;
				void WaitIdle() const override;
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