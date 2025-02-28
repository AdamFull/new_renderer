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

				auto GetFormat() const noexcept -> vk::Format
				{
					return create_info.format;
				}

				auto GetType() const noexcept -> vk::ImageViewType
				{
					return create_info.viewType;
				}

				auto GetSubresourceLayers() const noexcept -> vk::ImageSubresourceLayers
				{
					return vk::ImageSubresourceLayers(create_info.subresourceRange.aspectMask,
						create_info.subresourceRange.baseMipLevel,
						create_info.subresourceRange.baseArrayLayer,
						create_info.subresourceRange.layerCount);
				}

				auto GetSubresourceRange() const noexcept -> vk::ImageSubresourceRange
				{
					return create_info.subresourceRange;
				}

				auto GetAspectMask() const noexcept -> vk::ImageAspectFlags
				{
					return create_info.subresourceRange.aspectMask;
				}
			private:
				Device& device;

				vk::ImageViewCreateInfo create_info{};
			};
		}
	}
}