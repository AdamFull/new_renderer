#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class Semaphore final : public ISemaphore, public Resource<vk::Semaphore> {
	public:
		Semaphore(Device& device, bool timeline = false);
		~Semaphore();

		Semaphore(const Semaphore&) = delete;
		Semaphore(Semaphore&& other) = default;
		Semaphore& operator=(const Semaphore&) = delete;
		Semaphore& operator=(Semaphore&&) = default;

		void set_resource_name(std::string_view name) const noexcept override;
	private:
		Device& device;
	};
}