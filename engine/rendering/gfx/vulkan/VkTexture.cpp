#include "VkTexture.h"
#include "VkDevice.h"
#include "VkAllocator.h"

#include "VkSwapchain.h"

#include "VkUtil.h"

namespace engine
{
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
				VulkanSwapchain* vk_swapchain = static_cast<VulkanSwapchain*>(swapchain);
				auto& extent = vk_swapchain->GetExtent();

				TextureCreateInfo create_info{};
				create_info.width = extent.width;
				create_info.height = extent.height;
				create_info.usage = TextureUsageFlag::eRenderTarget;
				create_info.format = vk_swapchain->GetImageFormat();

				assert(image_index < vk_swapchain->GetImageCount());
				auto& image_handle = vk_swapchain->GetImage(image_index);

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

			void VulkanTexture::Reset()
			{
				srv->SetImageLayout(vk::ImageLayout::eUndefined);

				for (auto& subresource : subresources)
				{
					subresource->SetImageLayout(vk::ImageLayout::eUndefined);
				}
			}

			Image& VulkanTexture::GetImage()
			{
				return *image;
			}

			Image const& VulkanTexture::GetImage() const
			{
				return *image;
			}

			ImageView& VulkanTexture::GetSubresource(uint32_t subresource)
			{
				if (subresource == ~0u)
					return *srv;

				return *subresources[subresource];
			}

			ImageView const& VulkanTexture::GetSubresource(uint32_t subresource) const
			{
				if (subresource == ~0u)
					return *srv;

				return *subresources[subresource];
			}

			void VulkanTexture::SetImageLayout(vk::ImageLayout image_layout, uint32_t subresource)
			{
				if (subresource == ~0u)
				{
					srv->SetImageLayout(image_layout);

					for (auto& mip : subresources)
					{
						mip->SetImageLayout(image_layout);
					}
				}
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
}