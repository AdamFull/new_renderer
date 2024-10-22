#include <VkTexture.h>
#include <VkDevice.h>

#include <VkImage.h>

namespace gfx
{
	namespace vulkan
	{
		VulkanTexture::VulkanTexture(Device& device, const TextureCreateInfo& create_info) :
			device{ device }
		{

		}

		VulkanTexture::VulkanTexture(VulkanTexture&& other) :
			device{ device }
		{

		}

		VulkanTexture::~VulkanTexture()
		{

		}
	}
}