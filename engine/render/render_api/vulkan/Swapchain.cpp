#include <engine/render/render_api/vulkan/Swapchain.h>
#include <engine/render/render_api/vulkan/PhysicalDevice.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

#include <util/Logger.h>

#include <glm/glm.hpp>

namespace engine
{
	namespace render
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
					log_warning("(Swapchain) Image extent ({}, {}) not supported. Selecting ({}, {}).",
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

					log_warning("(Swapchain) Present mode '{}' not supported. Selecting '{}'.", vk::to_string(request_present_mode), vk::to_string(chosen_present_mode));
					return chosen_present_mode;
				}
				else
				{
					log_info("(Swapchain) Present mode selected: {}", to_string(request_present_mode));
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

					log_warning("(Swapchain) Surface format ({}) not supported. Selecting ({}).",
						vk::to_string(requested_surface_format.format) + ", " + vk::to_string(requested_surface_format.colorSpace),
						vk::to_string(chosen_surface_format.format) + ", " + vk::to_string(chosen_surface_format.colorSpace));
					return chosen_surface_format;
				}
				else
				{
					log_info("(Swapchain) Surface format selected: {}", vk::to_string(requested_surface_format.format) + ", " + vk::to_string(requested_surface_format.colorSpace));
					return requested_surface_format;
				}
			}

			vk::SurfaceTransformFlagBitsKHR ChooseTransform(vk::SurfaceTransformFlagBitsKHR request_transform, vk::SurfaceTransformFlagsKHR supported_transform, vk::SurfaceTransformFlagBitsKHR current_transform)
			{
				if (request_transform & supported_transform)
				{
					return request_transform;
				}

				log_warning("(Swapchain) Surface transform '{}' not supported. Selecting '{}'.", vk::to_string(request_transform), vk::to_string(current_transform));
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
					log_warning("(Swapchain) Composite alpha '{}' not supported. Selecting '{}.", vk::to_string(request_composite_alpha), vk::to_string(*chosen_composite_alpha_it));
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
						log_warning("(Swapchain) Image usage ({}) requested but not supported.", vk::to_string(flag));
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
					log_info("(Swapchain) Image usage flags: {}", usage_list);
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


			Swapchain::Swapchain(Swapchain& old_swapchain, const vk::Extent2D& extent) :
				Swapchain{ old_swapchain.render_device, old_swapchain.surface,
							 old_swapchain.properties.present_mode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 extent,
							 old_swapchain.properties.image_count,
							 old_swapchain.properties.pre_transform,
							 old_swapchain.image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			Swapchain::Swapchain(Swapchain& old_swapchain, const uint32_t image_count) :
				Swapchain{ old_swapchain.render_device, old_swapchain.surface,
							 old_swapchain.properties.present_mode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 old_swapchain.properties.extent,
							 image_count,
							 old_swapchain.properties.pre_transform,
							 old_swapchain.image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			Swapchain::Swapchain(Swapchain& old_swapchain, const std::set<vk::ImageUsageFlagBits>& image_usage_flags) :
				Swapchain{ old_swapchain.render_device, old_swapchain.surface,
							 old_swapchain.properties.present_mode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 old_swapchain.properties.extent,
							 old_swapchain.properties.image_count,
							 old_swapchain.properties.pre_transform,
							 image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			Swapchain::Swapchain(Swapchain& old_swapchain, const vk::Extent2D& extent, const vk::SurfaceTransformFlagBitsKHR transform) :
				Swapchain{ old_swapchain.render_device, old_swapchain.surface,
							 old_swapchain.properties.present_mode,
							 old_swapchain.present_mode_priority_list,
							 old_swapchain.surface_format_priority_list,
							 extent,
							 old_swapchain.properties.image_count,
							 transform,
							 old_swapchain.image_usage_flags,
							 old_swapchain.GetHandle() }
			{}

			Swapchain::Swapchain(RenderDevice& render_device, vk::SurfaceKHR surface,
				const vk::PresentModeKHR present_mode,
				const std::vector<vk::PresentModeKHR>& present_mode_priority_list,
				const std::vector<vk::SurfaceFormatKHR>& surface_format_priority_list,
				const vk::Extent2D& extent,
				uint32_t image_count,
				const vk::SurfaceTransformFlagBitsKHR transform,
				const std::set<vk::ImageUsageFlagBits>& image_usage_flags,
				vk::SwapchainKHR old_swapchain) :
				VulkanResource{ nullptr },
				render_device{ render_device },
				surface{ surface }
			{
				this->present_mode_priority_list = present_mode_priority_list;
				this->surface_format_priority_list = surface_format_priority_list;

				auto& physical_device = render_device.GetPhysicalDevice();

				std::vector<vk::SurfaceFormatKHR> surface_formats = Handle(physical_device).getSurfaceFormatsKHR(surface);
				log_info("Surface supports the following surface formats:");
				for (auto& surface_format : surface_formats)
				{
					log_info("  \t{}", vk::to_string(surface_format.format) + ", " + vk::to_string(surface_format.colorSpace));
				}

				std::vector<vk::PresentModeKHR> present_modes = Handle(physical_device).getSurfacePresentModesKHR(surface);
				log_info("Surface supports the following present modes:");
				for (auto& present_mode : present_modes)
				{
					log_info("  \t{}", to_string(present_mode));
				}

				// Choose best properties based on surface capabilities
				vk::SurfaceCapabilitiesKHR const surface_capabilities = Handle(physical_device).getSurfaceCapabilitiesKHR(surface);

				properties.old_swapchain = old_swapchain;
				properties.image_count = glm::clamp(image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount ? surface_capabilities.maxImageCount : std::numeric_limits<uint32_t>::max());
				properties.extent = ChooseExtent(extent, surface_capabilities.minImageExtent, surface_capabilities.maxImageExtent, surface_capabilities.currentExtent);
				properties.surface_format = ChooseSurfaceFormat(properties.surface_format, surface_formats, surface_format_priority_list);
				properties.array_layers = 1;

				vk::FormatProperties const format_properties = Handle(physical_device).getFormatProperties(properties.surface_format.format);
				this->image_usage_flags = ChooseImageUsage(image_usage_flags, surface_capabilities.supportedUsageFlags, format_properties.optimalTilingFeatures);

				properties.image_usage = CompositeImageFlags(this->image_usage_flags);
				properties.pre_transform = ChooseTransform(transform, surface_capabilities.supportedTransforms, surface_capabilities.currentTransform);
				properties.composite_alpha = ChooseCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit, surface_capabilities.supportedCompositeAlpha);
				properties.present_mode = ChoosePresentMode(present_mode, present_modes, present_mode_priority_list);

				vk::SwapchainCreateInfoKHR create_info{};
				create_info.surface = surface;
				create_info.minImageCount = properties.image_count;
				create_info.imageFormat = properties.surface_format.format;
				create_info.imageColorSpace = properties.surface_format.colorSpace;
				create_info.imageExtent = properties.extent;
				create_info.imageArrayLayers = properties.array_layers;
				create_info.imageUsage = properties.image_usage;
				create_info.imageSharingMode = vk::SharingMode::eExclusive;
				create_info.preTransform = properties.pre_transform;
				create_info.compositeAlpha = properties.composite_alpha;
				create_info.presentMode = properties.present_mode;
				create_info.clipped = VK_TRUE;
				create_info.oldSwapchain = properties.old_swapchain;

				handle = render_device.CreateSwapchain2(create_info, "swapchain");

				render_device.GetSwapchainImages(handle, images, image_count, "swapchain_image");

				// Create image views

				image_views.resize(image_count);

				vk::ImageViewCreateInfo image_view_ci{};
				image_view_ci.viewType = vk::ImageViewType::e2D;
				image_view_ci.format = properties.surface_format.format;
				image_view_ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
				image_view_ci.subresourceRange.baseMipLevel = 0;
				image_view_ci.subresourceRange.levelCount = 1;
				image_view_ci.subresourceRange.baseArrayLayer = 0;
				image_view_ci.subresourceRange.layerCount = 1;
				image_view_ci.components.r = vk::ComponentSwizzle::eR;
				image_view_ci.components.g = vk::ComponentSwizzle::eG;
				image_view_ci.components.b = vk::ComponentSwizzle::eB;
				image_view_ci.components.a = vk::ComponentSwizzle::eA;

				// TODO: Pass here debug utils for name image views
				for (int32_t i = 0; i < image_count; ++i)
				{
					image_view_ci.image = images[i];
					image_views[i] = render_device.CreateImageView(image_view_ci, std::format("swapchain_image_view_{}", i));
				}
			}

			Swapchain::~Swapchain()
			{
				render_device.Destroy(handle);

				// Destroy image views
				for (auto& image_view : image_views)
				{
					render_device.DestroyImageView(image_view);
				}
			}

			Swapchain::Swapchain(Swapchain&& other) :
				VulkanResource{ std::move(other) },
				render_device{ other.render_device },
				surface{ std::exchange(other.surface, nullptr) },
				images{ std::exchange(other.images, {}) },
				image_views{ std::exchange(other.image_views, {}) },
				properties{ std::exchange(other.properties, {}) },
				present_mode_priority_list{ std::exchange(other.present_mode_priority_list, {}) },
				surface_format_priority_list{ std::exchange(other.surface_format_priority_list, {}) },
				image_usage_flags{ std::move(other.image_usage_flags) }
			{}

			bool Swapchain::IsValid() const
			{
				return !!handle;
			}

			RenderDevice const& Swapchain::GetDevice() const
			{
				return render_device;
			}

			std::pair<vk::Result, uint32_t> Swapchain::AcquireNextImage(vk::Semaphore image_acquired_semaphore, vk::Fence fence) const
			{
				vk::ResultValue<uint32_t> rv = Handle(render_device.GetLogicalDevice()).acquireNextImageKHR(handle, std::numeric_limits<uint64_t>::max(), image_acquired_semaphore, fence);
				return std::make_pair(rv.result, rv.value);
			}

			const vk::Extent2D& Swapchain::GetExtent() const
			{
				return properties.extent;
			}

			vk::Format Swapchain::GetFormat() const
			{
				return properties.surface_format.format;
			}

			const std::vector<vk::Image>& Swapchain::GetImages() const
			{
				return images;
			}

			const std::vector<vk::ImageView>& Swapchain::GetImageViews() const
			{
				return image_views;
			}

			vk::SurfaceTransformFlagBitsKHR Swapchain::GetTransform() const
			{
				return properties.pre_transform;
			}

			vk::SurfaceKHR Swapchain::GetSurface() const
			{
				return surface;
			}

			vk::ImageUsageFlags Swapchain::GetUsage() const
			{
				return properties.image_usage;
			}

			vk::PresentModeKHR Swapchain::GetPresentMode() const
			{
				return properties.present_mode;
			}

			uint32_t Swapchain::GetImageCount() const
			{
				return properties.image_count;
			}
		}
	}
}