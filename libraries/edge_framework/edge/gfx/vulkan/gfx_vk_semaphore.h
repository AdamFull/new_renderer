#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class VulkanSemaphore final : public ISemaphore, public VulkanResource<vk::Semaphore>
	{
	public:
		VulkanSemaphore(Device& device, bool timeline = false);
		~VulkanSemaphore();

		VulkanSemaphore(const VulkanSemaphore&) = delete;
		VulkanSemaphore(VulkanSemaphore&& other) = default;
		VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
		VulkanSemaphore& operator=(VulkanSemaphore&&) = default;

		void set_resource_name(std::string_view name) const noexcept override;
	private:
		Device& device;
	};
}