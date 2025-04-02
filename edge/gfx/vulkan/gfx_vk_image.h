#pragma once

#include <memory>
#include "gfx_vk_allocator.h"

#include "../gfx_structures.h"

namespace edge::gfx::vulkan
{
	class Image : public Allocation, public VulkanResource<vk::Image> {
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
		[[nodiscard]] auto map() -> uint8_t*;

		auto get_type() const noexcept -> vk::ImageType {
			return create_info.imageType;
		}

		auto get_extent() const noexcept -> vk::Extent3D const& {
			return create_info.extent;
		}

		auto get_format() const noexcept -> vk::Format
		{
			return create_info.format;
		}

		auto get_sample_count() const noexcept -> vk::SampleCountFlagBits {
			return create_info.samples;
		}

		auto get_usage() const noexcept -> vk::ImageUsageFlags {
			return create_info.usage;
		}

		auto get_tiling() const noexcept -> vk::ImageTiling {
			return create_info.tiling;
		}

		auto get_subresource() const noexcept -> vk::ImageSubresource const& {
			return subresource;
		}

		auto get_mip_level_count() const noexcept -> uint32_t {
			return create_info.mipLevels;
		}

		auto get_array_layer_count() const noexcept -> uint32_t {
			return create_info.arrayLayers;
		}
	private:
		void initialize_create_info(const TextureCreateInfo& texture_create_info);
	private:
		bool is_external_handle{ false };
		vk::ImageCreateInfo create_info;
		vk::ImageSubresource subresource;
	};
}