#include "VkSampler.h"
#include "VkPhysicalDevice.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "VkUtil.h"

namespace edge::gfx::vulkan
{
	VulkanSampler::VulkanSampler(Device& device, const SamplerCreateInfo& create_info) :
		VulkanResource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::SamplerCreateInfo vk_create_info{};
		vk_create_info.minFilter = filter_to_vk(create_info.min_filter);
		vk_create_info.magFilter = filter_to_vk(create_info.mag_filter);
		vk_create_info.mipmapMode = mipmap_mode_to_vk(create_info.mip_map_mode);
		vk_create_info.addressModeU = address_mode_to_vk(create_info.address_u);
		vk_create_info.addressModeV = address_mode_to_vk(create_info.address_v);
		vk_create_info.addressModeW = address_mode_to_vk(create_info.address_w);
		vk_create_info.mipLodBias = create_info.mip_lod_bias;
		vk_create_info.minLod = create_info.min_lod;
		vk_create_info.maxLod = create_info.max_lod;
		vk_create_info.anisotropyEnable = create_info.max_anisotropy > 0.f;

		auto& physical = device.get_gpu();
		auto& properties = physical.get_properties();
		vk_create_info.maxAnisotropy = std::min(create_info.max_anisotropy, properties.limits.maxSamplerAnisotropy);

		vk_create_info.compareEnable = create_info.depth_compare != CompareOp::eNone;
		vk_create_info.compareOp = compare_op_to_vk(create_info.depth_compare);

		vk::Result result = device.get_handle().createSampler(&vk_create_info, allocation_callbacks, &handle_);
	}

	VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept :
		VulkanResource{ std::move(other) },
		device{ other.device } {
	}

	VulkanSampler::~VulkanSampler() {
		device.get_handle().destroySampler(handle_, allocation_callbacks);
	}

	void VulkanSampler::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}
}