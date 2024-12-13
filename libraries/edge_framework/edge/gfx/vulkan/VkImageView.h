#pragma once

#include "VkImage.h"
#include "../gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class ImageView : public VulkanResource<vk::ImageView>
			{
			public:
				ImageView(Image& image, const TextureCreateInfo& texture_create_info, uint32_t base_array_layer, uint32_t base_mip_level, uint32_t mip_count);

				ImageView(ImageView&) = delete;
				ImageView(ImageView&& other) noexcept;

				~ImageView() override;

				ImageView& operator=(const ImageView&) = delete;
				ImageView& operator=(ImageView&&) = delete;

				vk::Format GetFormat() const;
				vk::ImageViewType GetType() const;
				vk::ImageSubresourceLayers GetSubresourceLayers() const;
				vk::ImageSubresourceRange GetSubresourceRange() const;
				vk::ImageAspectFlags GetAspectMask() const;
			private:
				Device& device;

				vk::ImageViewCreateInfo create_info{};
			};
		}
	}
}