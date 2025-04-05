#include "gfx_vk_descriptor_set.h"
#include "gfx_vk_device.h"

namespace edge::gfx::vulkan
{
	DescriptorSet::DescriptorSet() :
		Resource{ nullptr, nullptr } {
	}

	DescriptorSet::DescriptorSet(vk::DescriptorSet handle, const PoolSizes& sizes) :
		Resource{ handle, nullptr },
		used_sizes{ sizes } {
	}

	DescriptorSet::DescriptorSet(DescriptorSet&& other) :
		Resource{ std::move(other) },
		used_sizes{ std::exchange(other.used_sizes, {}) } {
	}
}