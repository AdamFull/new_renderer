#pragma once

#include "VkImageView.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class VulkanTexture final : public ITexture
			{
			public:
				VulkanTexture(Allocator& allocator, const TextureCreateInfo& create_info);
				VulkanTexture(Allocator& allocator, ISwapchain* swapchain, uint32_t image_index);

				VulkanTexture(const VulkanTexture&) = delete;
				VulkanTexture(VulkanTexture&& other);
				~VulkanTexture() = default;

				VulkanTexture& operator=(const VulkanTexture&) = delete;
				VulkanTexture& operator=(VulkanTexture&&) = delete;

				[[nodiscard]] auto GetImage() noexcept -> Image&;
				[[nodiscard]] auto GetImage() const noexcept -> Image const&;

				[[nodiscard]] auto GetSubresource(uint32_t subresource = ~0u) -> ImageView&;
				[[nodiscard]] auto GetSubresource(uint32_t subresource = ~0u) const ->ImageView const&;

				void SetResourceName(std::string_view name) const noexcept override;

				[[nodiscard]] auto GetWidth() const noexcept -> uint32_t override
				{
					return image->GetExtent().width;
				}

				[[nodiscard]] auto GetHeight() const noexcept -> uint32_t override
				{
					return image->GetExtent().height;
				}

				[[nodiscard]] auto GetDepth() const noexcept -> uint32_t override
				{
					return image->GetExtent().depth;
				}

				[[nodiscard]] auto GetMipLevelCount() const noexcept -> uint32_t override
				{
					return image->GetMipLevelCount();
				}

				[[nodiscard]] auto GetArrayLayerCount() const noexcept -> uint32_t override
				{
					return image->GetArrayLayerCount();
				}

				[[nodiscard]] auto GetFormat() const noexcept -> Format override;
			private:
				Device& device;

				std::unique_ptr<Image> image{};
				std::unique_ptr<ImageView> srv;

				std::vector<std::unique_ptr<ImageView>> subresources{};
			};
		}
	}
}