#pragma once

#include <VkResource.h>
#include <GFXStructures.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;
		class Image;

		class ImageView : public VulkanResource<vk::ImageView>
		{
		public:
			ImageView(Image& image, const TextureCreateInfo& texture_create_info, uint32_t base_array_layer, uint32_t base_mip_level, uint32_t mip_count);

			ImageView(ImageView&) = delete;
			ImageView(ImageView&& other) noexcept;

			~ImageView() override;

			ImageView& operator=(const ImageView&) = delete;
			ImageView& operator=(ImageView&&) = delete;

			void SetImageLayout(vk::ImageLayout new_layout);

			vk::Format GetFormat() const;
			vk::ImageLayout GetImageLayout() const;
			vk::ImageViewType GetType() const;
			vk::ImageSubresourceLayers GetSubresourceLayers() const;
			vk::ImageSubresourceRange GetSubresourceRange() const;
		private:
			Device& device;
			bool external_handle{ false };

			vk::ImageViewCreateInfo create_info{};
			vk::ImageLayout layout{};
		};
	}
}