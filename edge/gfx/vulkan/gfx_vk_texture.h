#pragma once

#include "gfx_vk_image_view.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Texture final : public ITexture {
	public:
		Texture(Allocator& allocator, const TextureCreateInfo& create_info);
		Texture(Allocator& allocator, const ISwapchain& swapchain, uint32_t image_index);

		Texture(const Texture&) = delete;
		Texture(Texture&& other);
		~Texture() = default;

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

		[[nodiscard]] auto get_image() noexcept -> Image&;
		[[nodiscard]] auto get_image() const noexcept -> Image const&;

		[[nodiscard]] auto get_srv_view() -> ImageView&;
		[[nodiscard]] auto get_uav_view(uint32_t view_index = ~0u) -> ImageView&;

		[[nodiscard]] auto get_srv_view() const -> ImageView const&;
		[[nodiscard]] auto get_uav_view(uint32_t view_index = ~0u) const -> ImageView const&;

		void set_resource_name(std::string_view name) const noexcept override;

		auto get_usage() const noexcept -> TextureUsageFlags override {
			return usage;
		}

		auto get_width() const noexcept -> uint32_t override {
			return image->get_extent().width;
		}

		auto get_height() const noexcept -> uint32_t override {
			return image->get_extent().height;
		}

		auto get_depth() const noexcept -> uint32_t override {
			return image->get_extent().depth;
		}

		auto get_mip_level_count() const noexcept -> uint32_t override {
			return image->get_mip_level_count();
		}

		auto get_array_layer_count() const noexcept -> uint32_t override {
			return image->get_array_layer_count();
		}

		auto get_format() const noexcept -> Format override;
	private:
		Device& device;

		TextureUsageFlags usage{};
		std::unique_ptr<Image> image{};

		std::unique_ptr<ImageView> srv_view{};
		std::vector<std::unique_ptr<ImageView>> uav_views{};
	};
}