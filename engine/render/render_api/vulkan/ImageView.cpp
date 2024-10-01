#include <engine/render/render_api/vulkan/ImageView.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/Image.h>
#include <engine/render/render_api/vulkan/DebugUtils.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

#include <vulkan/vulkan_format_traits.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			ImageViewBuilder::ImageViewBuilder()
			{
				create_info.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
			}

			ImageViewBuilder& ImageViewBuilder::SetDebugName(const std::string& name)
			{
				debug_name = name;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetViewType(vk::ImageViewType view_type)
			{
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetFormat(vk::Format format)
			{
				create_info.format = format;
				create_info.subresourceRange.aspectMask = (strcmp(vk::componentName(format, 0), "D") == 0) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetComponentMapping(vk::ComponentMapping component_mapping)
			{
				create_info.components = component_mapping;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetSubresourceRange(const vk::ImageSubresourceRange& subresource_range)
			{
				create_info.subresourceRange = subresource_range;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetAspectMask(vk::ImageAspectFlags mask)
			{
				create_info.subresourceRange.aspectMask = mask;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetBaseMipLevel(uint32_t mip)
			{
				create_info.subresourceRange.baseMipLevel = mip;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetBaseArrayLayer(uint32_t layer)
			{
				create_info.subresourceRange.baseArrayLayer = layer;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetMipLevelCount(uint32_t count)
			{
				create_info.subresourceRange.levelCount = count;
				return *this;
			}

			ImageViewBuilder& ImageViewBuilder::SetArrayLayerlCount(uint32_t count)
			{
				create_info.subresourceRange.layerCount = count;
				return *this;
			}


			ImageView ImageViewBuilder::Build(VkRenderDevice& context, Image& image) const
			{
				Finalize(image);
				return ImageView(context, image, *this);
			}

			std::unique_ptr<ImageView> ImageViewBuilder::BuildUnique(VkRenderDevice& context, Image& image) const
			{
				Finalize(image);
				return std::make_unique<ImageView>(context, image, *this);
			}

			void ImageViewBuilder::Finalize(Image& image) const
			{
				auto& subresource_range = create_info.subresourceRange;
				if (create_info.format == vk::Format::eUndefined)
				{
					create_info.format = image.GetFormat();
					subresource_range.aspectMask = (strcmp(vk::componentName(create_info.format, 0), "D") == 0) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
				}

				if(subresource_range.levelCount == 0u)
					subresource_range.levelCount = image.GetSubresource().mipLevel;

				if (subresource_range.layerCount == 0u)
					subresource_range.layerCount = image.GetSubresource().arrayLayer;

				create_info.image = Handle(image);
			}


			ImageView::ImageView(VkRenderDevice& context, Image& image, ImageViewBuilder const& builder) :
				VulkanResource{ nullptr },
				device{ context.GetLogicalDevice() }, 
				format{ builder.create_info.format }, 
				subresource_range{ builder.create_info.subresourceRange }
			{
				device.CreateImageView(builder.create_info, handle);

				if (!builder.debug_name.empty())
				{
					context.GetDebugUtils().SetDebugName(handle, builder.debug_name.c_str());
				}
			}

			ImageView::ImageView(ImageView&& other) noexcept :
				VulkanResource{ std::move(other) }, 
				device{ other.device }, 
				format{ other.format }, 
				subresource_range{ other.subresource_range }
			{
			}

			ImageView::~ImageView()
			{
				if (handle)
				{
					device.Destroy(handle);
				}
			}

			vk::Format ImageView::GetFormat() const
			{
				return format;
			}

			vk::ImageSubresourceLayers ImageView::GetSubresourceLayers() const
			{
				return vk::ImageSubresourceLayers(subresource_range.aspectMask, subresource_range.baseMipLevel, subresource_range.baseArrayLayer, subresource_range.layerCount);
			}

			vk::ImageSubresourceRange ImageView::GetSubresourceRange() const
			{
				return subresource_range;
			}
		}
	}
}