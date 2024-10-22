#include <VkTexture.h>
#include <VkDevice.h>
#include <VkAllocator.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanTexture::VulkanTexture(Allocator& allocator, const TextureCreateInfo& create_info) :
			device{ allocator.GetDevice() }
		{
			// Create image handle
			image = std::make_unique<Image>(allocator, create_info);

			// Create base shader resource view
			srv = std::make_unique<ImageView>(*image, create_info, 0u, 0u, create_info.levels);

			subresources.resize(create_info.levels);
			for (u32 mip_index = 0u; mip_index < create_info.levels; ++mip_index)
			{
				subresources[mip_index] = std::make_unique<ImageView>(*image, create_info, 0u, mip_index, 1u);
			}
		}

		VulkanTexture::VulkanTexture(Allocator& allocator, Swapchain* swapchain, uint32_t image_index) :
			device{ allocator.GetDevice() }
		{
			assert(false && "NOT IMPLEMENTED");
			TextureCreateInfo create_info{};

			image = std::make_unique<Image>(allocator, create_info);
			srv = std::make_unique<ImageView>(*image, create_info, 0u, 0u, create_info.levels);
		}

		VulkanTexture::VulkanTexture(VulkanTexture&& other) :
			device{ device },
			image{ std::exchange(other.image, {}) },
			srv{ std::exchange(other.srv, {}) },
			subresources{ std::exchange(other.subresources, {}) }
		{

		}
	}
}