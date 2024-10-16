#pragma once

#include <set>

#include <engine/render/render_api/vulkan/VkResource.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;

			struct SwapchainProperties
			{
				vk::SwapchainKHR old_swapchain;
				uint32_t image_count{ 3 };
				vk::Extent2D extent;
				vk::SurfaceFormatKHR surface_format;
				uint32_t array_layers;
				vk::ImageUsageFlags image_usage;
				vk::SurfaceTransformFlagBitsKHR pre_transform;
				vk::CompositeAlphaFlagBitsKHR composite_alpha;
				vk::PresentModeKHR present_mode;
			};

			class Swapchain : public VulkanResource<vk::SwapchainKHR>
			{
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
				Swapchain(RenderDevice& device, vk::SurfaceKHR surface,
					const vk::PresentModeKHR present_mode,
					const std::vector<vk::PresentModeKHR>& present_mode_priority_list = { vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox },
					const std::vector<vk::SurfaceFormatKHR>& surface_format_priority_list = { {vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
																							 {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear} },
					const vk::Extent2D& extent = {}, uint32_t image_count = 3,
					const vk::SurfaceTransformFlagBitsKHR    transform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
					const std::set<vk::ImageUsageFlagBits>& image_usage_flags = { vk::ImageUsageFlagBits::eColorAttachment, vk::ImageUsageFlagBits::eTransferSrc },
					vk::SwapchainKHR                         old_swapchain = nullptr);

				Swapchain(const Swapchain&) = delete;

				Swapchain(Swapchain&& other);

				~Swapchain();

				Swapchain& operator=(const Swapchain&) = delete;

				Swapchain& operator=(Swapchain&&) = delete;

				bool IsValid() const;

				RenderDevice const& GetDevice() const;

				vk::Result AcquireNextImage(vk::Semaphore image_acquired_semaphore, vk::Fence fence, uint32_t& image_index) const;

				const vk::Extent2D& GetExtent() const;

				vk::Format GetFormat() const;

				vk::Image& GetImage(uint32_t index);
				const std::vector<vk::Image>& GetImages() const;

				vk::SurfaceTransformFlagBitsKHR GetTransform() const;

				vk::SurfaceKHR GetSurface() const;

				vk::ImageUsageFlags GetUsage() const;

				vk::PresentModeKHR GetPresentMode() const;

				uint32_t GetImageCount() const;

			private:
				RenderDevice& render_device;

				vk::SurfaceKHR surface;

				std::vector<vk::Image> images;

				SwapchainProperties properties;

				// A list of present modes in order of priority (vector[0] has high priority, vector[size-1] has low priority)
				std::vector<vk::PresentModeKHR> present_mode_priority_list;

				// A list of surface formats in order of priority (vector[0] has high priority, vector[size-1] has low priority)
				std::vector<vk::SurfaceFormatKHR> surface_format_priority_list;

				std::set<vk::ImageUsageFlagBits> image_usage_flags;
			};
		}
	}
}