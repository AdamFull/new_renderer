#include "gfx_vk_image.h"

#include "gfx_vk_device.h"
#include "gfx_vk_util.h"
#include "../../util/Logger.h"

namespace edge::gfx::vulkan
{
	Image::Image(Allocator& allocator, const TextureCreateInfo& texture_create_info) :
		Resource{ nullptr, allocator.get_allocation_callbacks() },
		Allocation{ allocator } {

		initialize_create_info(texture_create_info);
		handle_ = allocator.create_image(*this, create_info);
	}

	Image::Image(Allocator& allocator, const TextureCreateInfo& texture_create_info, vk::Image external_handle) :
		Resource{ external_handle, allocator.get_allocation_callbacks() },
		Allocation{ allocator },
		is_external_handle{ true } {
		initialize_create_info(texture_create_info);
	}

	Image::Image(Image&& other) noexcept :
		Resource{ std::move(other) },
		Allocation{ std::move(other) },
		create_info{ std::exchange(other.create_info, {}) },
		subresource{ std::exchange(other.subresource, {}) },
		is_external_handle{ std::exchange(other.is_external_handle, {}) } {
	}

	Image::~Image() {
		if (!is_external_handle) {
			allocator.destroy_image(*this, handle_);
		}
	}

	uint8_t* Image::map() {
		if (create_info.tiling != vk::ImageTiling::eLinear) {
			EDGE_LOGW("[VkImage] Mapping image memory that is not linear");
		}

		return map_memory();
	}

	void Image::initialize_create_info(const TextureCreateInfo& texture_create_info) {
		auto& device = allocator.get_device();

		alloc_create_info.usage = VMA_MEMORY_USAGE_UNKNOWN;

		bool is_3d = texture_create_info.depth > 1u;
		bool is_2d = texture_create_info.height > 1u;
		if (is_3d) {
			create_info.imageType = vk::ImageType::e3D;
		}
		else if (is_2d) {
			create_info.imageType = vk::ImageType::e2D;
		}
		else {
			create_info.imageType = vk::ImageType::e1D;
		}

		create_info.mipLevels = texture_create_info.levels;

		create_info.arrayLayers = texture_create_info.layers;
		bool is_array = texture_create_info.layers > 1u;
		bool is_cubemap = static_cast<bool>(texture_create_info.usage & TextureUsageFlag::eCubemap);
		if (is_array && is_cubemap) {
			create_info.arrayLayers = texture_create_info.layers * 6u;
		}
		else if (texture_create_info.usage & TextureUsageFlag::eCubemap) {
			create_info.arrayLayers = 6u;
		}

		create_info.extent = vk::Extent3D{ texture_create_info.width, texture_create_info.height, texture_create_info.depth };
		create_info.format = static_cast<vk::Format>(format_to_vk(texture_create_info.format));

		if (texture_create_info.usage & TextureUsageFlag::eRenderTarget) {
			create_info.usage |= ((is_depth_stencil_format(texture_create_info.format) || is_depth_only_format(texture_create_info.format)) ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment);
		}

		if (texture_create_info.usage & TextureUsageFlag::eTransferable)
			create_info.usage |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

		if (texture_create_info.usage & TextureUsageFlag::eSamplable)
			create_info.usage |= vk::ImageUsageFlagBits::eSampled;

		if (texture_create_info.usage & TextureUsageFlag::eStorage)
			create_info.usage |= vk::ImageUsageFlagBits::eStorage;

		create_info.flags = is_cubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits::eExtendedUsage;

		create_info.sharingMode = vk::SharingMode::eExclusive;

		auto queue_families = device.get_queue_family_indices();
		if (queue_families.size() > 1) {
			create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_families.size());
			create_info.pQueueFamilyIndices = &queue_families[0];
			create_info.sharingMode = vk::SharingMode::eConcurrent;
		}
	}
}