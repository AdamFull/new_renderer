#include "VkImageView.h"
#include "VkDevice.h"
#include "VkImage.h"

#include "VkUtil.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			ImageView::ImageView(Image& image, const TextureCreateInfo& texture_create_info, uint32_t base_array_layer, uint32_t base_mip_level, uint32_t mip_count) :
				VulkanResource{ nullptr, image.GetAllocationCallbacks() },
				device{ image.GetDevice() }
			{
				bool is_array = texture_create_info.layers > 1u;
				bool is_cubemap = static_cast<bool>(texture_create_info.usage & TextureUsageFlag::eCubemap);
				bool is_3d = texture_create_info.depth > 1u;

				// TODO: assert 3d arrays or 3d cubemaps

				create_info.subresourceRange.layerCount = texture_create_info.layers;
				if (is_array && is_cubemap)
				{
					create_info.subresourceRange.layerCount = create_info.subresourceRange.layerCount * 6u;
					create_info.viewType = vk::ImageViewType::eCubeArray;
				}
				else if (is_array)
				{
					create_info.viewType = vk::ImageViewType::e2DArray;
				}
				else if (is_cubemap)
				{
					create_info.subresourceRange.layerCount = 6u;
					create_info.viewType = vk::ImageViewType::eCube;
				}
				else if (is_3d)
				{
					create_info.viewType = vk::ImageViewType::e3D;
				}
				else
				{
					create_info.subresourceRange.layerCount = 1u;
					create_info.viewType = vk::ImageViewType::e2D;
				}

				create_info.format = image.GetFormat();
				create_info.image = image.GetHandle();

				if (IsDepthFormat(texture_create_info.format))
				{
					create_info.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eDepth;
				}

				if (IsDepthStencilFormat(texture_create_info.format))
				{
					create_info.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
				}

				if (create_info.subresourceRange.aspectMask == vk::ImageAspectFlagBits::eNone)
				{
					create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
				}

				if (create_info.subresourceRange.aspectMask & vk::ImageAspectFlagBits::eStencil)
				{
					create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
				}

				create_info.subresourceRange.baseArrayLayer = base_array_layer;
				create_info.subresourceRange.baseMipLevel = base_mip_level;
				create_info.subresourceRange.levelCount = mip_count;

				vk::Result result = Handle(device).createImageView(&create_info, allocation_callbacks, &handle);
			}

			ImageView::ImageView(ImageView&& other) noexcept :
				VulkanResource{ std::move(other) },
				device{ other.device },
				create_info{ other.create_info }
			{
			}

			ImageView::~ImageView()
			{
				Handle(device).destroyImageView(handle, allocation_callbacks);
			}
		}
	}
}