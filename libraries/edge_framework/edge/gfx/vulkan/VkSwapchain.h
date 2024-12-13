#pragma once

#include <set>

#include "VkResource.h"
#include "../gfx_base.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class VulkanSwapchain : public Swapchain, public VulkanResource<vk::SwapchainKHR>
			{
			public:
				/**
				 * @brief Constructor to create a swapchain by changing the extent
				 *        only and preserving the configuration from the old swapchain.
				 */
				VulkanSwapchain(VulkanSwapchain& old_swapchain, const vk::Extent2D& extent);

				/**
				 * @brief Constructor to create a swapchain by changing the image count
				 *        only and preserving the configuration from the old swapchain.
				 */
				VulkanSwapchain(VulkanSwapchain& old_swapchain, const uint32_t image_count);

				/**
				 * @brief Constructor to create a swapchain by changing the image usage
				 * only and preserving the configuration from the old swapchain.
				 */
				VulkanSwapchain(VulkanSwapchain& old_swapchain, const std::set<vk::ImageUsageFlagBits>& image_usage_flags);

				/**
				 * @brief Constructor to create a swapchain by changing the extent
				 *        and transform only and preserving the configuration from the old swapchain.
				 */
				VulkanSwapchain(VulkanSwapchain& swapchain, const vk::Extent2D& extent, const vk::SurfaceTransformFlagBitsKHR transform);

				/**
				 * @brief Constructor to create a swapchain.
				 */
				VulkanSwapchain(Device& device, vk::SurfaceKHR surface,
					const vk::PresentModeKHR present_mode,
					const std::vector<vk::PresentModeKHR>& present_mode_priority_list = { vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox },
					const std::vector<vk::SurfaceFormatKHR>& surface_format_priority_list = { {vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
																							 {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear} },
					const vk::Extent2D& extent = {}, uint32_t image_count = 1,
					const vk::SurfaceTransformFlagBitsKHR    transform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
					const std::set<vk::ImageUsageFlagBits>& image_usage_flags = { vk::ImageUsageFlagBits::eColorAttachment, vk::ImageUsageFlagBits::eTransferSrc },
					vk::SwapchainKHR                         old_swapchain = nullptr);

				VulkanSwapchain(const VulkanSwapchain&) = delete;

				VulkanSwapchain(VulkanSwapchain&& other);

				~VulkanSwapchain();

				VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
				VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;

				bool IsValid() const;

				Device const& GetDevice() const;

				void SetResourceName(std::string_view name) override;

				bool AcquireNextImage(Semaphore* signal_semaphore, Fence* fence, uint32_t* image_index) override;

				const vk::Extent2D& GetExtent() const;

				vk::Format GetFormat() const;

				vk::Image& GetImage(uint32_t index);
				const std::vector<vk::Image>& GetImages() const;

				vk::SurfaceTransformFlagBitsKHR GetTransform() const;

				vk::SurfaceKHR GetSurface() const;

				vk::ImageUsageFlags GetUsage() const;

				vk::PresentModeKHR GetPresentMode() const;

				uint32_t GetImageCount() const override;

				Format GetImageFormat() const override;

				const uint32_t& GetWidth() const override;
				const uint32_t& GetHeight() const override;

				void GetSurfaceExtent(uint32_t& width, uint32_t& height) override;

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
	}
}