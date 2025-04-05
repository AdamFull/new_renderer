#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class Fence final : public IFence, public Resource<vk::Fence> {
	public:
		Fence(Device& device);
		~Fence();

		void set_resource_name(std::string_view name) const noexcept override;

		void wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) override;
		void reset() override;

		Fence(const Fence&) = delete;
		Fence(Fence&& other) = default;
		Fence& operator=(const Fence&) = delete;
		Fence& operator=(Fence&&) = default;
	private:
		Device& device;
	};
}