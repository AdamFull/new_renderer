#pragma once

#include <memory>
#include <base/Allocator.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Image;
			class RenderDevice;

			class ImageBuilder : public AllocationBuilder<ImageBuilder, vk::ImageCreateInfo>
			{
			public:
				vk::Image external_handle{ nullptr };

				ImageBuilder(vk::Extent3D const& extent);
				ImageBuilder(vk::Extent2D const& extent);
				ImageBuilder(uint32_t width, uint32_t height = 1u, uint32_t depth = 1u);
				ImageBuilder& SetFormat(vk::Format format);
				ImageBuilder& SetImageType(vk::ImageType type);
				ImageBuilder& SetArrayLayers(uint32_t layers);
				ImageBuilder& SetMipLevels(uint32_t levels);
				ImageBuilder& SetSampleCount(vk::SampleCountFlagBits sample_count);
				ImageBuilder& SetTiling(vk::ImageTiling tiling);
				ImageBuilder& SetUsage(vk::ImageUsageFlags usage);
				ImageBuilder& SetFlags(vk::ImageCreateFlags flags);
				ImageBuilder& SetImplicitSharingMode();
				ImageBuilder& SetExternalHandle(vk::Image handle);

				Image Build(RenderDevice& render_device) const;
				std::unique_ptr<Image> BuildUnique(RenderDevice& render_device) const;
			};


			class Image : public Allocation, public VulkanResource<vk::Image>
			{
			public:
				Image(RenderDevice& context, ImageBuilder const& builder);

				Image(const Image&) = delete;

				Image(Image&& other) noexcept;

				~Image();

				Image& operator=(const Image&) = delete;

				Image& operator=(Image&&) = delete;

				/**
				 * @brief Maps vulkan memory to an host visible address
				 * @return Pointer to host visible memory
				 */
				uint8_t* Map();

				vk::ImageType GetType() const;
				const vk::Extent3D& GetExtent() const;
				vk::Format GetFormat() const;
				vk::SampleCountFlagBits GetSampleCount() const;
				vk::ImageUsageFlags GetUsage() const;
				vk::ImageTiling GetTiling() const;
				vk::ImageSubresource GetSubresource() const;
				uint32_t GetMipLevelCount() const;
				uint32_t GetArrayLayerCount() const;



			private:
				bool external_handle{ false };
				vk::ImageCreateInfo create_info;
				vk::ImageSubresource subresource;
			};
		}
	}
}