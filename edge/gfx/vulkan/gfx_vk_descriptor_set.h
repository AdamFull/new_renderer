#pragma once

#include "gfx_vk_descriptor_set_layout.h"

namespace edge::gfx::vulkan
{
	class Device;

	class DescriptorSet final : public Resource<vk::DescriptorSet> {
	public:
		friend class DescriptorPool;

		DescriptorSet();
		DescriptorSet(vk::DescriptorSet handle, const PoolSizes& sizes);

		DescriptorSet(const DescriptorSet&) = delete;

		DescriptorSet(DescriptorSet&& other);

		~DescriptorSet() = default;

		DescriptorSet& operator=(const DescriptorSet&) = delete;
		DescriptorSet& operator=(DescriptorSet&&) = delete;
	private:
		PoolSizes used_sizes;
	};
}