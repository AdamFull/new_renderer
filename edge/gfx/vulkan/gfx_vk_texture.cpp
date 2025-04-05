#include "gfx_vk_texture.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_allocator.h"

#include "gfx_vk_swapchain.h"

#include "gfx_vk_util.h"

namespace edge::gfx::vulkan
{
	VulkanTexture::VulkanTexture(Allocator& allocator, const TextureCreateInfo& create_info) :
		device{ allocator.get_device() } {
		// Create image handle
		image = std::make_unique<Image>(allocator, create_info);

		// Create base shader resource view
		srv = std::make_unique<ImageView>(*image, create_info, 0u, 0u, create_info.levels);

		subresources.resize(create_info.levels);
		for (uint32_t mip_index = 0u; mip_index < create_info.levels; ++mip_index) {
			subresources[mip_index] = std::make_unique<ImageView>(*image, create_info, 0u, mip_index, 1u);
		}
	}

	VulkanTexture::VulkanTexture(Allocator& allocator, ISwapchain* swapchain, uint32_t image_index) :
		device{ allocator.get_device() } {

		VulkanSwapchain* vk_swapchain = utl::cast<VulkanSwapchain*>(swapchain);
		auto& extent = vk_swapchain->get_extent();

		TextureCreateInfo create_info{};
		create_info.width = extent.width;
		create_info.height = extent.height;
		create_info.usage = TextureUsageFlag::eRenderTarget;
		create_info.format = vk_swapchain->get_image_format();

		assert(image_index < vk_swapchain->get_image_count());
		auto& image_handle = vk_swapchain->get_image(image_index);

		image = std::make_unique<Image>(allocator, create_info, image_handle);
		srv = std::make_unique<ImageView>(*image, create_info, 0u, 0u, create_info.levels);
	}

	VulkanTexture::VulkanTexture(VulkanTexture&& other) :
		device{ device },
		image{ std::exchange(other.image, {}) },
		srv{ std::exchange(other.srv, {}) },
		subresources{ std::exchange(other.subresources, {}) } {
	}

	auto VulkanTexture::get_image() noexcept -> Image& {
		return *image;
	}

	auto VulkanTexture::get_image() const noexcept -> Image const& {
		return *image;
	}

	auto VulkanTexture::get_subresource(uint32_t subresource) -> ImageView& {
		if (subresource == ~0u)
			return *srv;

		return *subresources[subresource];
	}

	void VulkanTexture::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();

		debug_utils.set_debug_name(image->get_handle(), name);
		debug_utils.set_debug_name(srv->get_handle(), name);

		uint32_t index{ 0u };
		for (const auto& subresource : subresources) {
			debug_utils.set_debug_name(subresource->get_handle(), std::format("{}_{}", name, index++));
		}
	}

	auto VulkanTexture::get_subresource(uint32_t subresource) const -> ImageView const& {
		if (subresource == ~0u)
			return *srv;

		return *subresources[subresource];
	}

	auto VulkanTexture::get_format() const noexcept -> Format {
		return format_from_vk(static_cast<uint32_t>(image->get_format()));
	}
}