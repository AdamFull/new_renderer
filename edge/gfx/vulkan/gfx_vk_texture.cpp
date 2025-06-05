#include "gfx_vk_texture.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_allocator.h"

#include "gfx_vk_swapchain.h"

#include "gfx_vk_util.h"

namespace edge::gfx::vulkan {
	vk::ImageAspectFlags get_image_aspect_mask_by_format(Format format) {
		vk::ImageAspectFlags mask{};

		if (is_depth_format(format)) {
			mask |= vk::ImageAspectFlagBits::eDepth;
		}

		if (is_depth_stencil_format(format)) {
			mask |= vk::ImageAspectFlagBits::eStencil;
		}

		if (mask == vk::ImageAspectFlagBits::eNone) {
			mask = vk::ImageAspectFlagBits::eColor;
		}

		if (mask & vk::ImageAspectFlagBits::eStencil) {
			mask = vk::ImageAspectFlagBits::eDepth;
		}

		return mask;
	}

	Texture::Texture(Allocator& allocator, const TextureCreateInfo& create_info) :
		device{ allocator.get_device() } {
		// Create image handle
		image = std::make_unique<Image>(allocator, create_info);

		bool is_array = create_info.layers > 1u;
		bool is_cubemap = static_cast<bool>(create_info.usage & TextureUsageFlag::eCubemap);
		bool is_3d = create_info.depth > 1u;
		bool is_2d = create_info.height > 1u;

		vk::ImageViewCreateInfo view_ci{};
		view_ci.image = image->get_handle();
		view_ci.format = image->get_format();
		view_ci.components.r = vk::ComponentSwizzle::eR;
		view_ci.components.g = vk::ComponentSwizzle::eG;
		view_ci.components.b = vk::ComponentSwizzle::eB;
		view_ci.components.a = vk::ComponentSwizzle::eA;

		view_ci.subresourceRange.aspectMask = get_image_aspect_mask_by_format(create_info.format);
		view_ci.subresourceRange.baseMipLevel = 0;
		view_ci.subresourceRange.levelCount = create_info.levels;
		view_ci.subresourceRange.layerCount = 1u;

		switch (image->get_type()) {
		case vk::ImageType::e1D: {
			view_ci.viewType = is_array ? vk::ImageViewType::e1DArray : vk::ImageViewType::e1D;
		} break;
		case vk::ImageType::e2D: {
			if (is_array && is_cubemap) {
				view_ci.subresourceRange.layerCount = view_ci.subresourceRange.layerCount * 6u;
				view_ci.viewType = vk::ImageViewType::eCubeArray;
			}
			else if (is_cubemap) {
				view_ci.subresourceRange.layerCount = 6u;
				view_ci.viewType = vk::ImageViewType::eCube;
			}
			else {
				view_ci.viewType = is_array ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
			}
		} break;
		case vk::ImageType::e3D: {
			view_ci.viewType =  vk::ImageViewType::e3D;
		} break;
		}

		// Create base shader resource view
		srv_view = std::make_unique<ImageView>(device, view_ci);

		if (create_info.usage & TextureUsageFlag::eStorage) {
			if (view_ci.viewType == vk::ImageViewType::eCube || view_ci.viewType == vk::ImageViewType::eCubeArray) {
				view_ci.viewType = vk::ImageViewType::e2DArray;
			}

			uav_views.resize(create_info.levels);

			view_ci.subresourceRange.levelCount = 1u;
			for (int32_t i = 0; i < static_cast<int32_t>(create_info.levels); ++i) {
				view_ci.subresourceRange.baseMipLevel = i;
				uav_views[i] = std::make_unique<ImageView>(device, view_ci);
			}
		}
	}

	Texture::Texture(Allocator& allocator, const ISwapchain& swapchain, uint32_t image_index) :
		device{ allocator.get_device() } {

		auto& vk_swapchain = static_cast<const Swapchain&>(swapchain);
		auto& extent = vk_swapchain.get_extent();

		TextureCreateInfo create_info{};
		create_info.width = extent.width;
		create_info.height = extent.height;
		create_info.usage = TextureUsageFlag::eRenderTarget;
		create_info.format = vk_swapchain.get_image_format();

		assert(image_index < vk_swapchain.get_image_count());
		auto& image_handle = vk_swapchain.get_image(image_index);

		image = std::make_unique<Image>(allocator, create_info, image_handle);

		vk::ImageViewCreateInfo view_ci{};
		view_ci.image = image->get_handle();
		view_ci.format = image->get_format();
		view_ci.components.r = vk::ComponentSwizzle::eR;
		view_ci.components.g = vk::ComponentSwizzle::eG;
		view_ci.components.b = vk::ComponentSwizzle::eB;
		view_ci.components.a = vk::ComponentSwizzle::eA;
		view_ci.viewType = vk::ImageViewType::e2D;

		view_ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		view_ci.subresourceRange.baseMipLevel = 0u;
		view_ci.subresourceRange.levelCount = 1u;
		view_ci.subresourceRange.layerCount = 1u;

		srv_view = std::make_unique<ImageView>(device, view_ci);
	}

	Texture::Texture(Texture&& other) :
		device{ device },
		image{ std::exchange(other.image, {}) },
		srv_view{ std::exchange(other.srv_view, {}) },
		uav_views{ std::exchange(other.uav_views, {}) } {
	}

	auto Texture::get_image() noexcept -> Image& {
		return *image;
	}

	auto Texture::get_image() const noexcept -> Image const& {
		return *image;
	}

	auto Texture::get_srv_view() -> ImageView& {
		return *srv_view;
	}

	auto Texture::get_uav_view(uint32_t view_index) -> ImageView& {
		return *uav_views[view_index != ~0u ? view_index : 0];
	}

	auto Texture::get_srv_view() const->ImageView const& {
		return *srv_view;
	}

	auto Texture::get_uav_view(uint32_t view_index) const->ImageView const& {
		return *uav_views[view_index];
	}

	void Texture::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();

		debug_utils.set_debug_name(image->get_handle(), name);
		debug_utils.set_debug_name(srv_view->get_handle(), name);

		uint32_t index{ 0u };
		for (const auto& uav_view : uav_views) {
			debug_utils.set_debug_name(uav_view->get_handle(), std::format("{}_{}", name, index++));
		}
	}

	auto Texture::get_format() const noexcept -> Format {
		return format_from_vk(static_cast<uint32_t>(image->get_format()));
	}
}