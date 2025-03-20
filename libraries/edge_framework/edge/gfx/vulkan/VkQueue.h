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

			class VulkanQueue final : public IQueue, public VulkanResource<vk::Queue> {
			public:
				VulkanQueue(QueueFamilyManager* family_manager, vk::Queue queue);
				VulkanQueue(VulkanQueue&& other);
				~VulkanQueue() = default;

				VulkanQueue(const VulkanQueue&) = delete;
				VulkanQueue& operator=(const VulkanQueue&) = delete;
				VulkanQueue& operator=(VulkanQueue&&) = delete;

				auto get_family() const noexcept -> QueueFamilyManager const& {
					return *family_manager;
				}

				void set_family(QueueFamilyManager* family) noexcept {
					family_manager = family;
				}

				auto is_used() const noexcept -> bool {
					return used;
				}

				void set_used(bool value) noexcept {
					used = value;
				}

				void set_resource_name(std::string_view name) const noexcept override;

				void submit(const SubmitInfo& submit_info, IFence* fence) const override;
				void present(const PresentInfo& present_info) const override;
				void wait_idle() const override;
			private:
				QueueFamilyManager* family_manager{ nullptr };

				bool used{ false };
			};
		}
	}
}