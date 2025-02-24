#pragma once

#include "VkResource.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;
			class QueueFamilyManager;

			class VulkanQueue : public Queue, public VulkanResource<vk::Queue>
			{
			public:
				VulkanQueue(QueueFamilyManager* family_manager, vk::Queue queue);
				VulkanQueue(VulkanQueue&& other);
				~VulkanQueue() = default;

				VulkanQueue(const VulkanQueue&) = delete;
				VulkanQueue& operator=(const VulkanQueue&) = delete;
				VulkanQueue& operator=(VulkanQueue&&) = delete;

				QueueFamilyManager const& GetFamily() const;
				void SetFamily(QueueFamilyManager* family);

				bool IsUsed() const;
				void SetUsed(bool value);

				void SetResourceName(std::string_view name) override;

				void Submit(const SubmitInfo& submit_info, Fence* fence) const override;
				void Present(const PresentInfo& present_info) const override;
				void WaitIdle() const override;
			private:
				QueueFamilyManager* family_manager{ nullptr };

				bool used{ false };
			};
		}
	}
}