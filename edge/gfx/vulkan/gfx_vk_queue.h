#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class Queue final : public IQueue, public Resource<vk::Queue> {
	public:
		Queue(Device& device, uint32_t family_index, uint32_t queue_index);
		Queue(Queue&& other);
		~Queue() = default;

		Queue(const Queue&) = delete;
		Queue& operator=(const Queue&) = delete;
		Queue& operator=(Queue&&) = delete;

		auto get_device() noexcept -> Device& {
			return device;
		}

		auto get_device() const noexcept -> const Device& {
			return device;
		}

		auto get_family_index() const noexcept -> uint32_t {
			return family_index;
		}

		void set_resource_name(std::string_view name) const noexcept override;

		void submit(const SubmitInfo& submit_info, IFence* fence) const override;
		void present(const PresentInfo& present_info) const override;
		void wait_idle() const override;
	private:
		Device& device;
		uint32_t family_index{ 0u };
		uint32_t queue_index{ 0u };
	};
}