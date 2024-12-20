#include "VkSwapchain.h"
#include "VkPhysicalDevice.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "VkSemaphore.h"
#include "VkFence.h"

#include "../../util/Logger.h"

#include <glm/glm.hpp>

#include "../../util/Profiler.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			vk::Extent2D ChooseExtent(vk::Extent2D request_extent, const vk::Extent2D& min_image_extent, const vk::Extent2D& max_image_extent, const vk::Extent2D& current_extent)
			{
				if (current_extent.width == 0xFFFFFFFF)
				{
					return request_extent;
				}

				if (request_extent.width < 1 || request_extent.height < 1)
				{
					EDGE_LOGW("(Swapchain) Image extent ({}, {}) not supported. Selecting ({}, {}).",
						request_extent.width, request_extent.height,
						current_extent.width, current_extent.height);
					return current_extent;
				}

				request_extent.width = glm::clamp(request_extent.width, min_image_extent.width, max_image_extent.width);
				request_extent.height = glm::clamp(request_extent.height, min_image_extent.height, max_image_extent.height);

				return request_extent;
			}

			vk::PresentModeKHR ChoosePresentMode(vk::PresentModeKHR request_present_mode, const std::vector<vk::PresentModeKHR>& available_present_modes, const std::vector<vk::PresentModeKHR>& present_mode_priority_list)
			{
				// Try to find the requested present mode in the available present modes
				auto const present_mode_it = std::find(available_present_modes.begin(), available_present_modes.end(), request_present_mode);
				if (present_mode_it == available_present_modes.end())
				{
					// If the requested present mode isn't found, then try to find a mode from the priority list
					auto const chosen_present_mode_it =
						std::find_if(present_mode_priority_list.begin(), present_mode_priority_list.end(),
							[&available_present_modes](vk::PresentModeKHR present_mode) { return std::find(available_present_modes.begin(), available_present_modes.end(), present_mode) != available_present_modes.end(); });

					// If nothing found, always default to FIFO
					vk::PresentModeKHR const chosen_present_mode = (chosen_present_mode_it != present_mode_priority_list.end()) ? *chosen_present_mode_it : vk::PresentModeKHR::eFifo;

					EDGE_LOGW("(Swapchain) Present mode '{}' not supported. Selecting '{}'.", vk::to_string(request_present_mode), vk::to_string(chosen_present_mode));
					return chosen_present_mode;
				}
				else
				{
					EDGE_LOGI("(Swapchain) Present mode selected: {}", to_string(request_present_mode));
					return request_present_mode;
				}
			}

			vk::SurfaceFormatKHR ChooseSurfaceFormat(const vk::SurfaceFormatKHR requested_surface_format, const std::vector<vk::SurfaceFormatKHR>& available_surface_formats, const std::vector<vk::SurfaceFormatKHR>& surface_format_priority_list)
			{
				// Try to find the requested surface format in the available surface formats
				auto const surface_format_it = std::find(available_surface_formats.begin(), available_surface_formats.end(), requested_surface_format);

				// If the requested surface format isn't found, then try to request a format from the priority list
				if (surface_format_it == available_surface_formats.end())
				{
					auto const chosen_surface_format_it =
						std::find_if(surface_format_priority_list.begin(), surface_format_priority_list.end(),
							[&available_surface_formats](vk::SurfaceFormatKHR surface_format) { return std::find(available_surface_formats.begin(), available_surface_formats.end(), surface_format) != available_surface_formats.end(); });

					// If nothing found, default to the first available format
					vk::SurfaceFormatKHR const& chosen_surface_format = (chosen_surface_format_it != surface_format_priority_list.end()) ? *chosen_surface_format_it : available_surface_formats[0];

					EDGE_LOGW("(Swapchain) Surface format ({}) not supported. Selecting ({}).",
						vk::to_string(requested_surface_format.format) + ", " + vk::to_string(requested_surface_format.colorSpace),
						vk::to_string(chosen_surface_format.format) + ", " + vk::to_string(chosen_surface_format.colorSpace));
					return chosen_surface_format;
				}
				else
				{
					EDGE_LOGI("(Swapchain) Surface format selected: {}", vk::to_string(requested_surface_format.format) + ", " + vk::to_string(requested_surface_format.colorSpace));
					return requested_surface_format;
				}
			}

			vk::SurfaceTransformFlagBitsKHR ChooseTransform(vk::SurfaceTransformFlagBitsKHR request_transform, vk::SurfaceTransformFlagsKHR supported_transform, vk::SurfaceTransformFlagBitsKHR current_transform)
			{
				if (request_transform & supported_transform)
				{
					return request_transform;
				}

				EDGE_LOGW("(Swapchain) Surface transform '{}' not supported. Selecting '{}'.", vk::to_string(request_transform), vk::to_string(current_transform));
				return current_transform;
			}

			vk::CompositeAlphaFlagBitsKHR ChooseCompositeAlpha(vk::CompositeAlphaFlagBitsKHR request_composite_alpha, vk::CompositeAlphaFlagsKHR supported_composite_alpha)
			{
				if (request_composite_alpha & supported_composite_alpha)
				{
					return request_composite_alpha;
				}

				static const std::vector<vk::CompositeAlphaFlagBitsKHR> composite_alpha_priority_list = { vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
																										 vk::CompositeAlphaFlagBitsKHR::ePostMultiplied, vk::CompositeAlphaFlagBitsKHR::eInherit };

				auto const chosen_composite_alpha_it = std::find_if(composite_alpha_priority_list.begin(), composite_alpha_priority_list.end(),
					[&supported_composite_alpha](vk::CompositeAlphaFlagBitsKHR composite_alpha) { return composite_alpha & supported_composite_alpha; });

				if (chosen_composite_alpha_it == composite_alpha_priority_list.end())
				{
					throw std::runtime_error("No compatible composite alpha found.");
				}
				else
				{
					EDGE_LOGW("(Swapchain) Composite alpha '{}' not supported. Selecting '{}.", vk::to_string(request_composite_alpha), vk::to_string(*chosen_composite_alpha_it));
					return *chosen_composite_alpha_it;
				}
			}

			bool ValidateFormatFeature(vk::ImageUsageFlagBits image_usage, vk::FormatFeatureFlags supported_features)
			{
				return (image_usage != vk::ImageUsageFlagBits::eStorage) || (supported_features & vk::FormatFeatureFlagBits::eStorageImage);
			}

			std::set<vk::ImageUsageFlagBits> ChooseImageUsage(const std::set<vk::ImageUsageFlagBits>& requested_image_usage_flags, vk::ImageUsageFlags supported_image_usage, vk::FormatFeatureFlags supported_features)
			{
				std::set<vk::ImageUsageFlagBits> validated_image_usage_flags;
				for (auto flag : requested_image_usage_flags)
				{
					if ((flag & supported_image_usage) && ValidateFormatFeature(flag, supported_features))
					{
						validated_image_usage_flags.insert(flag);
					}
					else
					{
						EDGE_LOGW("(Swapchain) Image usage ({}) requested but not supported.", vk::to_string(flag));
					}
				}

				if (validated_image_usage_flags.empty())
				{
					// Pick the first format from list of defaults, if supported
					static const std::vector<vk::ImageUsageFlagBits> image_usage_priority_list = { vk::ImageUsageFlagBits::eColorAttachment, vk::ImageUsageFlagBits::eStorage, vk::ImageUsageFlagBits::eSampled, vk::ImageUsageFlagBits::eTransferDst };

					auto const priority_list_it = std::find_if(image_usage_priority_list.begin(), image_usage_priority_list.end(),
						[&supported_image_usage, &supported_features](auto const image_usage) { return ((image_usage & supported_image_usage) && ValidateFormatFeature(image_usage, supported_features)); });
					if (priority_list_it != image_usage_priority_list.end())
					{
						validated_image_usage_flags.insert(*priority_list_it);
					}
				}

				if (validated_image_usage_flags.empty())
				{
					throw std::runtime_error("No compatible image usage found.");
				}
				else
				{
					// Log image usage flags used
					std::string usage_list;
					for (vk::ImageUsageFlagBits image_usage : validated_image_usage_flags)
					{
						usage_list += to_string(image_usage) + " ";
					}
					EDGE_LOGI("(Swapchain) Image usage flags: {}", usage_list);
				}

				return validated_image_usage_flags;
			}

			vk::ImageUsageFlags CompositeImageFlags(std::set<vk::ImageUsageFlagBits>& image_usage_flags)
			{
				vk::ImageUsageFlags image_usage;
				for (auto flag : image_usage_flags)
				{
					image_usage |= flag;
				}
				return image_usage;
			}


			VulkanSwapchain::VulkanSwapchain(VulkanSwapchain& old_swapchain, const vk::Extent2D& extent) :
				VulkanSwapchain{ old_swapchain.device, old_swapchain.surface,
							 old_swapchain.create_info.presentMode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 extent,
							 old_swapchain.create_info.minImageCount,
							 old_swapchain.create_info.preTransform,
							 old_swapchain.image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			VulkanSwapchain::VulkanSwapchain(VulkanSwapchain& old_swapchain, const uint32_t image_count) :
				VulkanSwapchain{ old_swapchain.device, old_swapchain.surface,
							 old_swapchain.create_info.presentMode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 old_swapchain.create_info.imageExtent,
							 image_count,
							 old_swapchain.create_info.preTransform,
							 old_swapchain.image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			VulkanSwapchain::VulkanSwapchain(VulkanSwapchain& old_swapchain, const std::set<vk::ImageUsageFlagBits>& image_usage_flags) :
				VulkanSwapchain{ old_swapchain.device, old_swapchain.surface,
							 old_swapchain.create_info.presentMode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 old_swapchain.create_info.imageExtent,
							 old_swapchain.create_info.minImageCount,
							 old_swapchain.create_info.preTransform,
							 image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			VulkanSwapchain::VulkanSwapchain(VulkanSwapchain& old_swapchain, const vk::Extent2D& extent, const vk::SurfaceTransformFlagBitsKHR transform) :
				VulkanSwapchain{ old_swapchain.device, old_swapchain.surface,
							 old_swapchain.create_info.presentMode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 extent,
							 old_swapchain.create_info.minImageCount,
							 transform,
							 old_swapchain.image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			VulkanSwapchain::VulkanSwapchain(Device& device, vk::SurfaceKHR surface,
				const vk::PresentModeKHR present_mode,
				const std::vector<vk::PresentModeKHR>& present_mode_priority_list,
				const std::vector<vk::SurfaceFormatKHR>& surface_format_priority_list,
				const vk::Extent2D& extent,
				uint32_t image_count,
				const vk::SurfaceTransformFlagBitsKHR transform,
				const std::set<vk::ImageUsageFlagBits>& image_usage_flags,
				vk::SwapchainKHR old_swapchain) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device },
				surface{ surface }
			{
				this->present_mode_priority_list = present_mode_priority_list;
				this->surface_format_priority_list = surface_format_priority_list;

				auto& physical_device = device.GetGpu();

				std::vector<vk::SurfaceFormatKHR> surface_formats = Handle(physical_device).getSurfaceFormatsKHR(surface);
				EDGE_LOGI("Surface supports the following surface formats:");
				for (auto& surface_format : surface_formats)
				{
					EDGE_LOGI("  \t{}", vk::to_string(surface_format.format) + ", " + vk::to_string(surface_format.colorSpace));
				}

				std::vector<vk::PresentModeKHR> present_modes = Handle(physical_device).getSurfacePresentModesKHR(surface);
				EDGE_LOGI("Surface supports the following present modes:");
				for (auto& present_mode : present_modes)
				{
					EDGE_LOGI("  \t{}", to_string(present_mode));
				}

				// Choose best properties based on surface capabilities
				vk::SurfaceCapabilitiesKHR const surface_capabilities = Handle(physical_device).getSurfaceCapabilitiesKHR(surface);

				create_info.oldSwapchain = old_swapchain;
				create_info.minImageCount = glm::clamp(image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount ? surface_capabilities.maxImageCount : std::numeric_limits<uint32_t>::max());
				create_info.imageExtent = ChooseExtent(extent, surface_capabilities.minImageExtent, surface_capabilities.maxImageExtent, surface_capabilities.currentExtent);
				auto surface_format = ChooseSurfaceFormat({}, surface_formats, surface_format_priority_list);
				create_info.imageFormat = surface_format.format;
				create_info.imageArrayLayers = 1;

				vk::FormatProperties const format_properties = Handle(physical_device).getFormatProperties(create_info.imageFormat);
				this->image_usage_flags = ChooseImageUsage(image_usage_flags, surface_capabilities.supportedUsageFlags, format_properties.optimalTilingFeatures);

				create_info.imageUsage = CompositeImageFlags(this->image_usage_flags);
				create_info.preTransform = ChooseTransform(transform, surface_capabilities.supportedTransforms, surface_capabilities.currentTransform);
				create_info.compositeAlpha = ChooseCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit, surface_capabilities.supportedCompositeAlpha);
				create_info.presentMode = ChoosePresentMode(present_mode, present_modes, present_mode_priority_list);

				create_info.surface = surface;
				create_info.imageFormat = surface_format.format;
				create_info.imageColorSpace = surface_format.colorSpace;
				create_info.imageSharingMode = vk::SharingMode::eExclusive;
				create_info.clipped = VK_TRUE;

				vk::Result result = Handle(device).createSwapchainKHR(&create_info, allocation_callbacks, &handle);

				images.resize(image_count);
				result = Handle(device).getSwapchainImagesKHR(handle, &image_count, images.data());
			}

			VulkanSwapchain::~VulkanSwapchain()
			{
				Handle(device).destroySwapchainKHR(handle, allocation_callbacks);
			}

			VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& other) :
				VulkanResource{ std::move(other) },
				device{ other.device },
				surface{ std::exchange(other.surface, nullptr) },
				images{ std::exchange(other.images, {}) },
				//image_views{ std::exchange(other.image_views, {}) },
				create_info{ std::exchange(other.create_info, {}) },
				present_mode_priority_list{ std::exchange(other.present_mode_priority_list, {}) },
				surface_format_priority_list{ std::exchange(other.surface_format_priority_list, {}) },
				image_usage_flags{ std::move(other.image_usage_flags) }
			{}

			bool VulkanSwapchain::IsValid() const
			{
				return !!handle;
			}

			Device const& VulkanSwapchain::GetDevice() const
			{
				return device;
			}

			void VulkanSwapchain::SetResourceName(std::string_view name)
			{
				auto& debug_utils = device.GetDebugUtils();
				debug_utils.SetDebugName(handle, name);
			}

			bool VulkanSwapchain::AcquireNextImage(Semaphore* signal_semaphore, Fence* fence, uint32_t* image_index)
			{
				EDGE_PROFILE_ZONE_SCOPED;

				vk::Semaphore image_acquired_semaphore{ nullptr };
				if (signal_semaphore)
				{
					auto* vk_semaphore = static_cast<VulkanSemaphore*>(signal_semaphore);
					image_acquired_semaphore = vk_semaphore->GetHandle();
				}

				vk::Fence wait_fence{ nullptr };
				if (fence)
				{
					auto* vk_fence = static_cast<VulkanFence*>(fence);
					wait_fence = vk_fence->GetHandle();
				}

				vk::Result result;
				try
				{
					result = Handle(device).acquireNextImageKHR(handle, std::numeric_limits<uint64_t>::max(), image_acquired_semaphore, wait_fence, image_index);
				}
				catch (vk::OutOfDateKHRError& /*err*/)
				{
					result = vk::Result::eErrorOutOfDateKHR;
				}

				return result == vk::Result::eSuccess;
			}

			const vk::Extent2D& VulkanSwapchain::GetExtent() const
			{
				return create_info.imageExtent;
			}

			vk::Format VulkanSwapchain::GetFormat() const
			{
				return create_info.imageFormat;
			}

			vk::Image& VulkanSwapchain::GetImage(uint32_t index)
			{
				return images[index];
			}

			const std::vector<vk::Image>& VulkanSwapchain::GetImages() const
			{
				return images;
			}

			vk::SurfaceTransformFlagBitsKHR VulkanSwapchain::GetTransform() const
			{
				return create_info.preTransform;
			}

			vk::SurfaceKHR VulkanSwapchain::GetSurface() const
			{
				return surface;
			}

			vk::ImageUsageFlags VulkanSwapchain::GetUsage() const
			{
				return create_info.imageUsage;
			}

			vk::PresentModeKHR VulkanSwapchain::GetPresentMode() const
			{
				return create_info.presentMode;
			}

			uint32_t VulkanSwapchain::GetImageCount() const
			{
				return create_info.minImageCount;
			}

			Format VulkanSwapchain::GetImageFormat() const
			{
				return FormatFromVk(static_cast<uint32_t>(create_info.imageFormat));
			}

			const uint32_t& VulkanSwapchain::GetWidth() const
			{
				return create_info.imageExtent.width;
			}

			const uint32_t& VulkanSwapchain::GetHeight() const
			{
				return create_info.imageExtent.height;
			}

			void VulkanSwapchain::GetSurfaceExtent(uint32_t& width, uint32_t& height)
			{
				auto& gpu = device.GetGpu();
				vk::SurfaceCapabilitiesKHR surface_properties = Handle(gpu).getSurfaceCapabilitiesKHR(surface);
				width = surface_properties.currentExtent.width;
				height = surface_properties.currentExtent.height;
			}
		}
	}
}