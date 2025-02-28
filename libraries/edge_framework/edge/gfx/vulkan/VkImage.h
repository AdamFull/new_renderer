#pragma once

#include <memory>
#include "VkAllocator.h"

#include "../gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Image : public Allocation, public VulkanResource<vk::Image>
			{
			public:
				Image(Allocator& allocator, const TextureCreateInfo& create_info);
				Image(Allocator& allocator, const TextureCreateInfo& create_info, vk::Image external_handle);
				Image(Image&& other) noexcept;
				~Image();

				Image(const Image&) = delete;
				Image& operator=(const Image&) = delete;
				Image& operator=(Image&&) = delete;

				/**
				 * @brief Maps vulkan memory to an host visible address
				 * @return Pointer to host visible memory
				 */
				[[nodiscard]] auto Map() -> uint8_t*;

				auto GetType() const noexcept -> vk::ImageType
				{
					return create_info.imageType;
				}

				auto GetExtent() const noexcept -> vk::Extent3D const&
				{
					return create_info.extent;
				}

				auto GetFormat() const noexcept -> vk::Format
				{
					return create_info.format;
				}

				auto GetSampleCount() const noexcept -> vk::SampleCountFlagBits
				{
					return create_info.samples;
				}

				auto GetUsage() const noexcept -> vk::ImageUsageFlags
				{
					return create_info.usage;
				}

				auto GetTiling() const noexcept -> vk::ImageTiling
				{
					return create_info.tiling;
				}

				auto GetSubresource() const noexcept -> vk::ImageSubresource const&
				{
					return subresource;
				}

				auto GetMipLevelCount() const noexcept -> uint32_t
				{
					return create_info.mipLevels;
				}

				auto GetArrayLayerCount() const noexcept -> uint32_t
				{
					return create_info.arrayLayers;
				}
			private:
				void InitializeCreateInfo(const TextureCreateInfo& texture_create_info);
			private:
				bool is_external_handle{ false };
				vk::ImageCreateInfo create_info;
				vk::ImageSubresource subresource;
			};
		}
	}
}