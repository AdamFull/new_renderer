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

			class VulkanSemaphore final : public ISemaphore, public VulkanResource<vk::Semaphore>
			{
			public:
				VulkanSemaphore(Device& device);
				~VulkanSemaphore();

				VulkanSemaphore(const VulkanSemaphore&) = delete;
				VulkanSemaphore(VulkanSemaphore&& other) = default;
				VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
				VulkanSemaphore& operator=(VulkanSemaphore&&) = default;

				void SetResourceName(std::string_view name) const noexcept override;
			private:
				Device& device;
			};
		}
	}
}