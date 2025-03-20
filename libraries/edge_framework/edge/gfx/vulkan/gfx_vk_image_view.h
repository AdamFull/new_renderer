#pragma once

#include "gfx_vk_image.h"
#include "../gfx_structures.h"

namespace edge::gfx::vulkan
{
	class ImageView : public VulkanResource<vk::ImageView> {
	public:
		ImageView(Image& image, const TextureCreateInfo& texture_create_info, uint32_t base_array_layer, uint32_t base_mip_level, uint32_t mip_count);

		ImageView(ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;

		~ImageView() override;

		ImageView& operator=(const ImageView&) = delete;
		ImageView& operator=(ImageView&&) = delete;

		auto get_format() const noexcept -> vk::Format {
			return create_info.format;
		}

		auto get_type() const noexcept -> vk::ImageViewType {
			return create_info.viewType;
		}

		auto get_subresource_layers() const noexcept -> vk::ImageSubresourceLayers {
			return vk::ImageSubresourceLayers(create_info.subresourceRange.aspectMask,
				create_info.subresourceRange.baseMipLevel,
				create_info.subresourceRange.baseArrayLayer,
				create_info.subresourceRange.layerCount);
		}

		auto get_subresource_range() const noexcept -> vk::ImageSubresourceRange {
			return create_info.subresourceRange;
		}

		auto get_aspect_mask() const noexcept -> vk::ImageAspectFlags {
			return create_info.subresourceRange.aspectMask;
		}
	private:
		Device& device;

		vk::ImageViewCreateInfo create_info{};
	};
}