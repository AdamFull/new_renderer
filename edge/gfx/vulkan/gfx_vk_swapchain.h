#pragma once

#include <set>

#include "gfx_vk_resource.h"
#include "../gfx_base.h"

namespace edge::gfx::vulkan
{
	class Device;

	class Swapchain final : public ISwapchain, public Resource<vk::SwapchainKHR> {
	public:
		/**
		 * @brief Constructor to create a swapchain by changing the extent
		 *        only and preserving the configuration from the old swapchain.
		 */
		Swapchain(Swapchain& old_swapchain, const vk::Extent2D& extent);

		/**
		 * @brief Constructor to create a swapchain by changing the image count
		 *        only and preserving the configuration from the old swapchain.
		 */
		Swapchain(Swapchain& old_swapchain, const uint32_t image_count);

		/**
		 * @brief Constructor to create a swapchain by changing the image usage
		 * only and preserving the configuration from the old swapchain.
		 */
		Swapchain(Swapchain& old_swapchain, const std::set<vk::ImageUsageFlagBits>& image_usage_flags);

		/**
		 * @brief Constructor to create a swapchain by changing the extent
		 *        and transform only and preserving the configuration from the old swapchain.
		 */
		Swapchain(Swapchain& swapchain, const vk::Extent2D& extent, const vk::SurfaceTransformFlagBitsKHR transform);

		/**
		 * @brief Constructor to create a swapchain.
		 */
		Swapchain(Device& device, vk::SurfaceKHR surface,
			const vk::PresentModeKHR present_mode,
			const std::vector<vk::PresentModeKHR>& present_mode_priority_list = { vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox },
			const std::vector<vk::SurfaceFormatKHR>& surface_format_priority_list = { {vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
																					 {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear} },
			const vk::Extent2D& extent = {}, uint32_t image_count = 1,
			const vk::SurfaceTransformFlagBitsKHR    transform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
			const std::set<vk::ImageUsageFlagBits>& image_usage_flags = { vk::ImageUsageFlagBits::eColorAttachment, vk::ImageUsageFlagBits::eTransferSrc },
			vk::SwapchainKHR                         old_swapchain = nullptr);

		Swapchain(const Swapchain&) = delete;

		Swapchain(Swapchain&& other);

		~Swapchain();

		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(Swapchain&&) = delete;

		[[nodiscard]] auto is_valid() const noexcept -> bool {
			return !!handle_;
		}

		Device const& get_device() const noexcept {
			return device;
		}

		void set_resource_name(std::string_view name) const noexcept override;

		[[nodiscard]] auto acquire_next_image(ISemaphore* signal_semaphore, IFence* fence, uint32_t* image_index) -> bool override;

		auto get_extent() const noexcept -> vk::Extent2D const& {
			return create_info.imageExtent;
		}

		auto get_format() const noexcept -> vk::Format {
			return create_info.imageFormat;
		}

		[[nodiscard]] auto get_image(uint32_t index) noexcept -> vk::Image& {
			return images[index];
		}

		[[nodiscard]] auto get_images() const noexcept -> std::vector<vk::Image> const& {
			return images;
		}

		auto get_transform() const noexcept -> vk::SurfaceTransformFlagBitsKHR {
			return create_info.preTransform;
		}

		[[nodiscard]] auto get_surface() const noexcept -> vk::SurfaceKHR {
			return surface;
		}

		auto get_usage() const noexcept -> vk::ImageUsageFlags {
			return create_info.imageUsage;
		}

		auto get_present_mode() const noexcept -> vk::PresentModeKHR {
			return create_info.presentMode;
		}

		auto get_image_count() const noexcept -> uint32_t override;

		[[nodiscard]] auto get_image_format() const noexcept -> Format override;

		auto get_width() const noexcept -> uint32_t const& override;
		auto get_height() const noexcept -> uint32_t const& override;

		void get_surface_extent(uint32_t& width, uint32_t& height) override;

	private:
		Device& device;

		vk::SurfaceKHR surface;

		std::vector<vk::Image> images;
		vk::SwapchainCreateInfoKHR create_info{};

		// A list of present modes in order of priority (vector[0] has high priority, vector[size-1] has low priority)
		std::vector<vk::PresentModeKHR> present_mode_priority_list;

		// A list of surface formats in order of priority (vector[0] has high priority, vector[size-1] has low priority)
		std::vector<vk::SurfaceFormatKHR> surface_format_priority_list;

		std::set<vk::ImageUsageFlagBits> image_usage_flags;
	};
}