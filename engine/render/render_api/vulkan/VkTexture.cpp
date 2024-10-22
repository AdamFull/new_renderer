#include <VkTexture.h>
#include <VkDevice.h>
#include <VkAllocator.h>

#include <VkImage.h>
#include <VkImageView.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanTexture::VulkanTexture(Allocator& allocator, const TextureCreateInfo& create_info) :
			device{ allocator.GetDevice() }
		{
			image = std::make_unique<Image>(allocator, create_info);

			srv = std::make_unique<ImageView>(*image, create_info, 0u, 0u, create_info.levels);
		}

		VulkanTexture::VulkanTexture(VulkanTexture&& other) :
			device{ device },
			image{ std::exchange(other.image, {}) },
			srv{ std::exchange(other.srv, {}) }
		{

		}

		VulkanTexture::~VulkanTexture()
		{

		}
	}
}