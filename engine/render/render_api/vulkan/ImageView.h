#pragma once

#include <engine/render/render_api/vulkan/Allocator.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Image;
			class ImageView;
			class RenderDevice;

			struct ImageViewBuilder
			{
				std::string debug_name;
				vk::ImageView external_handle;
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
				ImageViewBuilder& SetExternalHandle(vk::ImageView handle);

				ImageView Build(RenderDevice& render_device, Image& image) const;
				std::unique_ptr<ImageView> BuildUnique(RenderDevice& render_device, Image& image) const;

			private:
				void Finalize(Image& image) const;
			};

			class ImageView : public VulkanResource<vk::ImageView>
			{
			public:
				ImageView(RenderDevice& render_device, Image& image, ImageViewBuilder const& builder);

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
				RenderDevice& render_device;
				bool external_handle{ false };

				vk::Format format;
				vk::ImageLayout layout;
				vk::ImageViewType type;
				vk::ImageSubresourceRange subresource_range;
			};
		}
	}
}