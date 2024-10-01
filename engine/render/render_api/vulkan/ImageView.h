#pragma once

#include <engine/render/render_api/vulkan/Allocator.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Device;
			class Image;
			class ImageView;
			class VkRenderDevice;

			struct ImageViewBuilder
			{
				std::string debug_name;
				mutable vk::ImageViewCreateInfo create_info;

				ImageViewBuilder();
			
				ImageViewBuilder& SetDebugName(const std::string& name);
				ImageViewBuilder& SetViewType(vk::ImageViewType view_type);
				ImageViewBuilder& SetFormat(vk::Format format);
				ImageViewBuilder& SetComponentMapping(vk::ComponentMapping component_mapping);
				ImageViewBuilder& SetSubresourceRange(const vk::ImageSubresourceRange& subresource_range);
				ImageViewBuilder& SetAspectMask(vk::ImageAspectFlags mask);
				ImageViewBuilder& SetBaseMipLevel(uint32_t mip);
				ImageViewBuilder& SetBaseArrayLayer(uint32_t layer);
				ImageViewBuilder& SetMipLevelCount(uint32_t count);
				ImageViewBuilder& SetArrayLayerlCount(uint32_t count);

				ImageView Build(VkRenderDevice& context, Image& image) const;
				std::unique_ptr<ImageView> BuildUnique(VkRenderDevice& context, Image& image) const;

			private:
				void Finalize(Image& image) const;
			};

			class ImageView : public VulkanResource<vk::ImageView>
			{
			public:
				ImageView(VkRenderDevice& context, Image& image, ImageViewBuilder const& builder);

				ImageView(ImageView&) = delete;
				ImageView(ImageView&& other) noexcept;

				~ImageView() override;

				ImageView& operator=(const ImageView&) = delete;
				ImageView& operator=(ImageView&&) = delete;

				vk::Format GetFormat() const;
				vk::ImageSubresourceLayers GetSubresourceLayers() const;
				vk::ImageSubresourceRange GetSubresourceRange() const;
			private:
				Device& device;

				vk::Format format;
				vk::ImageSubresourceRange subresource_range;
			};
		}
	}
}