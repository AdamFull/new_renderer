#pragma once

#include "VkImageView.h"
#include <gfx_base.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class VulkanTexture final : public Texture
			{
			public:
				VulkanTexture(Allocator& allocator, const TextureCreateInfo& create_info);
				VulkanTexture(Allocator& allocator, Swapchain* swapchain, uint32_t image_index);

				VulkanTexture(const VulkanTexture&) = delete;
				VulkanTexture(VulkanTexture&& other);
				~VulkanTexture() = default;

				VulkanTexture& operator=(const VulkanTexture&) = delete;
				VulkanTexture& operator=(VulkanTexture&&) = delete;

				Image& GetImage();
				Image const& GetImage() const;

				ImageView& GetSubresource(uint32_t subresource = ~0u);
				ImageView const& GetSubresource(uint32_t subresource = ~0u) const;

				void SetResourceName(const char* name) override;

				uint32_t GetWidth() const override;
				uint32_t GetHeight() const override;
				uint32_t GetDepth() const override;

				uint32_t GetMipLevelCount() const override;
				uint32_t GetArrayLayerCount() const override;

				Format GetFormat() const override;
			private:
				Device& device;

				std::unique_ptr<Image> image{};
				std::unique_ptr<ImageView> srv;

				std::vector<std::unique_ptr<ImageView>> subresources{};
			};
		}
	}
}