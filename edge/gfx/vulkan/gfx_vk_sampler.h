#pragma once

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class Sampler final : public ISampler, public Resource<vk::Sampler> {
	public:
		Sampler(Device& device, const SamplerCreateInfo& create_info);
		~Sampler();

		Sampler(Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;

		Sampler& operator=(const Sampler&) = delete;
		Sampler& operator=(Sampler&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		auto hash() const noexcept -> uint32_t override {
			return self_hash;
		}
	private:
		Device& device;

		uint32_t self_hash{ 0u };
	};
}