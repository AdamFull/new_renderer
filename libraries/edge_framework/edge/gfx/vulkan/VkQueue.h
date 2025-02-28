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

			class VulkanQueue final : public IQueue, public VulkanResource<vk::Queue>
			{
			public:
				VulkanQueue(QueueFamilyManager* family_manager, vk::Queue queue);
				VulkanQueue(VulkanQueue&& other);
				~VulkanQueue() = default;

				VulkanQueue(const VulkanQueue&) = delete;
				VulkanQueue& operator=(const VulkanQueue&) = delete;
				VulkanQueue& operator=(VulkanQueue&&) = delete;

				auto GetFamily() const noexcept -> QueueFamilyManager const&
				{
					return *family_manager;
				}

				void SetFamily(QueueFamilyManager* family) noexcept
				{
					family_manager = family;
				}

				auto IsUsed() const noexcept -> bool
				{
					return used;
				}

				void SetUsed(bool value) noexcept
				{
					used = value;
				}

				void SetResourceName(std::string_view name) const noexcept override;

				void Submit(const SubmitInfo& submit_info, IFence* fence) const override;
				void Present(const PresentInfo& present_info) const override;
				void WaitIdle() const override;
			private:
				QueueFamilyManager* family_manager{ nullptr };

				bool used{ false };
			};
		}
	}
}