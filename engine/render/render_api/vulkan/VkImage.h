#pragma once

#include <memory>
#include <VkAllocator.h>

#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class Image : public Allocation, public VulkanResource<vk::Image>
		{
		public:
			Image(Allocator& allocator, const TextureCreateInfo& create_info);
			Image(Image&& other) noexcept;
			~Image();

			Image(const Image&) = delete;
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