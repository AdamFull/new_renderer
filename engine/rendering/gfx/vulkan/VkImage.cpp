#include "VkImage.h"

#include "VkUtil.h"
#include <util/Logger.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			Image::Image(Allocator& allocator, const TextureCreateInfo& texture_create_info) :
				VulkanResource{ nullptr, allocator.GetAllocationCallbacks() },
				Allocation{ allocator }
			{
				InitializeCreateInfo(texture_create_info);
				handle = allocator.CreateImage(*this, create_info);
			}

			Image::Image(Allocator& allocator, const TextureCreateInfo& texture_create_info, vk::Image external_handle) :
				VulkanResource{ external_handle, allocator.GetAllocationCallbacks() },
				Allocation{ allocator },
				is_external_handle{ true }
			{
				InitializeCreateInfo(texture_create_info);
			}

			Image::Image(Image&& other) noexcept :
				VulkanResource{ std::move(other) },
				Allocation{ std::move(other) },
				create_info{ std::exchange(other.create_info, {}) },
				subresource{ std::exchange(other.subresource, {}) },
				is_external_handle{ std::exchange(other.is_external_handle, {}) }
			{
			}

			Image::~Image()
			{
				if (!is_external_handle)
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

			void Image::InitializeCreateInfo(const TextureCreateInfo& texture_create_info)
			{
				alloc_create_info.usage = vma::MemoryUsage::eUnknown;

				bool is_3d = texture_create_info.depth > 1u;
				if (is_3d)
				{
					create_info.imageType = vk::ImageType::e3D;
				}
				else if (texture_create_info.height != 0u || texture_create_info.height != 0u)
				{
					create_info.imageType = vk::ImageType::e2D;
				}
				else
				{
					create_info.imageType = vk::ImageType::e1D;
				}

				create_info.mipLevels = texture_create_info.levels;

				create_info.arrayLayers = texture_create_info.layers;
				bool is_array = texture_create_info.layers > 1u;
				bool is_cubemap = static_cast<bool>(texture_create_info.usage & TextureUsageFlag::eCubemap);
				if (is_array && is_cubemap)
				{
					create_info.arrayLayers = texture_create_info.layers * 6u;
				}
				else if (texture_create_info.usage & TextureUsageFlag::eCubemap)
				{
					create_info.arrayLayers = 6u;
				}

				create_info.extent = vk::Extent3D{ texture_create_info.width, texture_create_info.height, texture_create_info.depth };
				create_info.format = static_cast<vk::Format>(FormatToVk(texture_create_info.format));

				if (texture_create_info.usage & TextureUsageFlag::eRenderTarget)
				{
					create_info.usage |= ((IsDepthStencilFormat(texture_create_info.format) || IsDepthOnlyFormat(texture_create_info.format)) ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment);
				}

				if (texture_create_info.usage & TextureUsageFlag::eTransferable)
					create_info.usage |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

				if (texture_create_info.usage & TextureUsageFlag::eSamplable)
					create_info.usage |= vk::ImageUsageFlagBits::eSampled;

				if (texture_create_info.usage & TextureUsageFlag::eStorage)
					create_info.usage |= vk::ImageUsageFlagBits::eStorage;

				create_info.sharingMode = vk::SharingMode::eExclusive;
				create_info.flags = is_cubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;
			}
		}
	}
}