#include <VkImage.h>

#include <util/Logger.h>

namespace gfx
{
	namespace vulkan
	{
		Image::Image(Allocator& allocator, const TextureCreateInfo& texture_create_info) :
			VulkanResource{ nullptr, allocator.GetAllocationCallbacks() },
			Allocation{ allocator }
		{
			
		}

		Image::Image(Image&& other) noexcept :
			VulkanResource{ std::move(other) },
			Allocation{ std::move(other) },
			create_info(std::exchange(other.create_info, {})),
			subresource(std::exchange(other.subresource, {}))
		{
		}

		Image::~Image()
		{
			if (!external_handle)
			{
				allocator.DestroyImage(*this, handle);
			}
		}

		uint8_t* Image::Map()
		{
			if (create_info.tiling != vk::ImageTiling::eLinear)
			{
				log_warning("Mapping image memory that is not linear");
			}
			
			return MapMemory();
		}

		vk::ImageType Image::GetType() const
		{
			return create_info.imageType;
		}

		const vk::Extent3D& Image::GetExtent() const
		{
			return create_info.extent;
		}

		vk::Format Image::GetFormat() const
		{
			return create_info.format;
		}

		vk::SampleCountFlagBits Image::GetSampleCount() const
		{
			return create_info.samples;
		}

		vk::ImageUsageFlags Image::GetUsage() const
		{
			return create_info.usage;
		}

		vk::ImageTiling Image::GetTiling() const
		{
			return create_info.tiling;
		}

		vk::ImageSubresource Image::GetSubresource() const
		{
			return subresource;
		}

		uint32_t Image::GetMipLevelCount() const
		{
			return create_info.mipLevels;
		}

		uint32_t Image::GetArrayLayerCount() const
		{
			return create_info.arrayLayers;
		}
	}
}