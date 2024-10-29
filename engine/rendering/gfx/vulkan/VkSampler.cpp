#include "VkSampler.h"
#include "VkPhysicalDevice.h"
#include "VkDevice.h"

#include "VkUtil.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanSampler::VulkanSampler(Device& device, const SamplerCreateInfo& create_info) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vk::SamplerCreateInfo vk_create_info{};
				vk_create_info.minFilter = FilterToVk(create_info.min_filter);
				vk_create_info.magFilter = FilterToVk(create_info.mag_filter);
				vk_create_info.mipmapMode = MipmapModeToVk(create_info.mip_map_mode);
				vk_create_info.addressModeU = AddressModeToVk(create_info.address_u);
				vk_create_info.addressModeV = AddressModeToVk(create_info.address_v);
				vk_create_info.addressModeW = AddressModeToVk(create_info.address_w);
				vk_create_info.mipLodBias = create_info.mip_lod_bias;
				vk_create_info.minLod = create_info.min_lod;
				vk_create_info.maxLod = create_info.max_lod;
				vk_create_info.anisotropyEnable = create_info.max_anisotropy > 0.f;

				auto& physical = device.GetGpu();
				auto& properties = physical.GetProperties();
				vk_create_info.maxAnisotropy = std::min(create_info.max_anisotropy, properties.limits.maxSamplerAnisotropy);

				vk_create_info.compareEnable = create_info.depth_compare != CompareOp::eNone;
				vk_create_info.compareOp = CompareOpToVk(create_info.depth_compare);

				vk::Result result = Handle(device).createSampler(&vk_create_info, allocation_callbacks, &handle);
			}

			VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept :
				VulkanResource{ std::move(other) },
				device{ other.device }
			{
			}

			VulkanSampler::~VulkanSampler()
			{
				Handle(device).destroySampler(handle, allocation_callbacks);
			}
		}
	}
}