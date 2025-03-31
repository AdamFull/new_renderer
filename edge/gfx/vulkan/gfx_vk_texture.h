#pragma once

#include "gfx_vk_image_view.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class VulkanTexture final : public ITexture {
	public:
		VulkanTexture(Allocator& allocator, const TextureCreateInfo& create_info);
		VulkanTexture(Allocator& allocator, ISwapchain* swapchain, uint32_t image_index);

		VulkanTexture(const VulkanTexture&) = delete;
		VulkanTexture(VulkanTexture&& other);
		~VulkanTexture() = default;

		VulkanTexture& operator=(const VulkanTexture&) = delete;
		VulkanTexture& operator=(VulkanTexture&&) = delete;

		[[nodiscard]] auto get_image() noexcept -> Image&;
		[[nodiscard]] auto get_image() const noexcept -> Image const&;

		[[nodiscard]] auto get_subresource(uint32_t subresource = ~0u) -> ImageView&;
		[[nodiscard]] auto get_subresource(uint32_t subresource = ~0u) const->ImageView const&;

		void set_resource_name(std::string_view name) const noexcept override;

		[[nodiscard]] auto get_width() const noexcept -> uint32_t override {
			return image->get_extent().width;
		}

		[[nodiscard]] auto get_height() const noexcept -> uint32_t override {
			return image->get_extent().height;
		}

		[[nodiscard]] auto get_depth() const noexcept -> uint32_t override {
			return image->get_extent().depth;
		}

		[[nodiscard]] auto get_mip_level_count() const noexcept -> uint32_t override {
			return image->get_mip_level_count();
		}

		[[nodiscard]] auto get_array_layer_count() const noexcept -> uint32_t override {
			return image->get_array_layer_count();
		}

		[[nodiscard]] auto get_format() const noexcept -> Format override;
	private:
		Device& device;

		std::unique_ptr<Image> image{};
		std::unique_ptr<ImageView> srv;

		std::vector<std::unique_ptr<ImageView>> subresources{};
	};
}