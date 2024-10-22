#include <VkTexture.h>
#include <VkDevice.h>
#include <VkAllocator.h>

#include <VkSwapchain.h>

#include <VkUtil.h>

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
			VulkanSwapchain* vkswapchain = static_cast<VulkanSwapchain*>(swapchain);
			auto& extent = vkswapchain->GetExtent();

			TextureCreateInfo create_info{};
			create_info.width = extent.width;
			create_info.height = extent.height;
			create_info.usage = TextureUsageFlag::eRenderTarget;
			// TODO: IMAGE FORMAT
			assert(false && "NOT IMPLEMENTED");

			assert(image_index < vkswapchain->GetImageCount());
			auto& image_handle = vkswapchain->GetImage(image_index);

			image = std::make_unique<Image>(allocator, create_info, image_handle);
			srv = std::make_unique<ImageView>(*image, create_info, 0u, 0u, create_info.levels);
		}

		VulkanTexture::VulkanTexture(VulkanTexture&& other) :
			device{ device },
			image{ std::exchange(other.image, {}) },
			srv{ std::exchange(other.srv, {}) },
			subresources{ std::exchange(other.subresources, {}) }
		{
		}

		uint32_t VulkanTexture::GetWidth() const
		{
			return image->GetExtent().width;
		}

		uint32_t VulkanTexture::GetHeight() const
		{
			return image->GetExtent().height;
		}

		uint32_t VulkanTexture::GetDepth() const
		{
			return image->GetExtent().depth;
		}

		uint32_t VulkanTexture::GetMipLevelCount() const
		{
			return image->GetMipLevelCount();
		}

		uint32_t VulkanTexture::GetArrayLayerCount() const
		{
			return image->GetArrayLayerCount();
		}

		Format VulkanTexture::GetFormat() const
		{
			return FormatFromVk(static_cast<uint32_t>(image->GetFormat()));
		}
	}
}