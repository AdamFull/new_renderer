#include "VkTexture.h"
#include "VkDevice.h"
#include "VkDebug.h"
#include "VkAllocator.h"

#include "VkSwapchain.h"

#include "VkUtil.h"

namespace edge
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
				for (uint32_t mip_index = 0u; mip_index < create_info.levels; ++mip_index)
				{
					subresources[mip_index] = std::make_unique<ImageView>(*image, create_info, 0u, mip_index, 1u);
				}
			}

			VulkanTexture::VulkanTexture(Allocator& allocator, ISwapchain* swapchain, uint32_t image_index) :
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

			auto VulkanTexture::GetImage() noexcept -> Image&
			{
				return *image;
			}

			auto VulkanTexture::GetImage() const noexcept -> Image const&
			{
				return *image;
			}

			auto VulkanTexture::GetSubresource(uint32_t subresource) -> ImageView&
			{
				if (subresource == ~0u)
					return *srv;

				return *subresources[subresource];
			}

			void VulkanTexture::SetResourceName(std::string_view name) const noexcept
			{
				auto& debug_utils = device.GetDebugUtils();

				debug_utils.SetDebugName(image->GetHandle(), name);

				debug_utils.SetDebugName(srv->GetHandle(), name);

				uint32_t index{ 0u };
				for (const auto& subresource : subresources)
				{
					debug_utils.SetDebugName(subresource->GetHandle(), std::format("{}_{}", name, index++));
				}
			}

			auto VulkanTexture::GetSubresource(uint32_t subresource) const -> ImageView const&
			{
				if (subresource == ~0u)
					return *srv;

				return *subresources[subresource];
			}

			auto VulkanTexture::GetFormat() const noexcept -> Format
			{
				return FormatFromVk(static_cast<uint32_t>(image->GetFormat()));
			}
		}
	}
}