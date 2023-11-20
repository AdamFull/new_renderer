#pragma once

#include <VkResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			// Helper functions
			inline bool check_contains_single(const std::string& s1, const std::vector<std::string>& rhs)
			{
				return std::find_if(rhs.begin(), rhs.end(),
					[s1](const std::string& s2) { return s1 == s2; }) != rhs.end();
			}

			// Check that second vector contains all items from first one
			inline bool check_contains_all(const std::vector<std::string>& lhs, const std::vector<std::string>& rhs)
			{
				return std::all_of(lhs.begin(), lhs.end(),
					[&rhs](const std::string& s1) { return check_contains_single(s1, rhs); });
			}

			inline std::vector<const char*> get_instance_extension_names()
			{
#if defined( VK_USE_PLATFORM_WIN32_KHR )
				return std::vector<const char*>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
#else
				return std::vector<const char*>{};
#endif
			}

			inline vk::ImageUsageFlags get_texture_usage_flags(const FVkTexture& texture)
			{
				vk::ImageUsageFlags usageFlags{};

				if (texture.isRenderTarget)
					usageFlags |= ((texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT) ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment);

				if (texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT)
					usageFlags |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

				if (texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT)
					usageFlags |= vk::ImageUsageFlagBits::eSampled;

				if (texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT)
					usageFlags |= vk::ImageUsageFlagBits::eStorage;

				return usageFlags;
			}

			inline vk::ImageAspectFlags get_texture_aspect_flags(const FVkTexture& texture)
			{
				vk::ImageAspectFlags aspectFlags{ vk::ImageAspectFlagBits::eColor };

				if (texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT)
					aspectFlags = vk::ImageAspectFlagBits::eDepth;

				return aspectFlags;
			}

			inline vk::PrimitiveTopology get_primitive_topology(uint32_t mask)
			{
				switch (mask)
				{
				case SHADER_PRIMITIVE_TOPOLOGY_POINT_LIST: return vk::PrimitiveTopology::ePointList; break;
				case SHADER_PRIMITIVE_TOPOLOGY_LINE_LIST: return vk::PrimitiveTopology::eLineList; break;
				case SHADER_PRIMITIVE_TOPOLOGY_LINE_STRIP: return vk::PrimitiveTopology::eLineStrip; break;
				case SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return vk::PrimitiveTopology::eTriangleList; break;
				case SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return vk::PrimitiveTopology::eTriangleStrip; break;
				case SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN: return vk::PrimitiveTopology::eTriangleFan; break;
				}
			}

			inline vk::ShaderStageFlagBits get_shader_stage(uint32_t mask)
			{
				switch (mask)
				{
				case 1: return vk::ShaderStageFlagBits::eVertex; break;
				case 2: return vk::ShaderStageFlagBits::eTessellationControl; break;
				case 3: return vk::ShaderStageFlagBits::eTessellationEvaluation; break;
				case 4: return vk::ShaderStageFlagBits::eGeometry; break;
				case 5: return vk::ShaderStageFlagBits::eFragment; break;
				case 6: return vk::ShaderStageFlagBits::eCompute; break;
				case 7: return vk::ShaderStageFlagBits::eRaygenKHR; break;
				case 8: return vk::ShaderStageFlagBits::eIntersectionKHR; break;
				case 9: return vk::ShaderStageFlagBits::eAnyHitKHR; break;
				case 10: return vk::ShaderStageFlagBits::eClosestHitKHR; break;
				case 11: return vk::ShaderStageFlagBits::eMissKHR; break;
				case 12: return vk::ShaderStageFlagBits::eCallableKHR; break;
				case 13: return vk::ShaderStageFlagBits::eTaskEXT; break;
				case 14: return vk::ShaderStageFlagBits::eMeshEXT; break;
				}

				return vk::ShaderStageFlagBits::eAll;
			}

			inline vk::CullModeFlags get_cull_mode_flags(uint32_t mask)
			{
				vk::CullModeFlags flags{};

				if (mask & SHADER_CULL_MODE_NONE)
					flags |= vk::CullModeFlagBits::eNone;
				if (mask & SHADER_CULL_MODE_FRONT_BIT)
					flags |= vk::CullModeFlagBits::eFront;
				if (mask & SHADER_CULL_MODE_BACK_BIT)
					flags |= vk::CullModeFlagBits::eBack;

				return flags;
			}

			inline vk::FrontFace get_front_face(uint32_t mask)
			{
				if (mask == SHADER_FRONT_FACE_CCW)
					return vk::FrontFace::eCounterClockwise;

				return vk::FrontFace::eClockwise;
			}

			inline FVkPhysicalDevice make_physical_device(vk::PhysicalDevice device)
			{
				FVkPhysicalDevice current_device{};
				current_device.handle = device;
				current_device.properties = device.getProperties();

				auto extension_props = device.enumerateDeviceExtensionProperties();
				for (auto& extension : extension_props)
					current_device.avaliableExtensions.push_back(extension.extensionName);

				return current_device;
			}

			inline std::vector<vk::SampleCountFlagBits> get_avaliable_sample_count(const FVkPhysicalDevice& device)
			{
				std::vector<vk::SampleCountFlagBits> avaliable_samples{ vk::SampleCountFlagBits::e1 };

				auto sample_count_flags = device.properties.limits.framebufferColorSampleCounts & device.properties.limits.framebufferDepthSampleCounts;
				if (sample_count_flags & vk::SampleCountFlagBits::e64)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e64);
				if (sample_count_flags & vk::SampleCountFlagBits::e32)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e32);
				if (sample_count_flags & vk::SampleCountFlagBits::e16)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e16);
				if (sample_count_flags & vk::SampleCountFlagBits::e8)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e8);
				if (sample_count_flags & vk::SampleCountFlagBits::e4)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e4);
				if (sample_count_flags & vk::SampleCountFlagBits::e2)
					avaliable_samples.push_back(vk::SampleCountFlagBits::e2);

				return avaliable_samples;
			}

			inline bool is_sample_count_supported(const FVkPhysicalDevice& device, vk::SampleCountFlagBits sample_count)
			{
				auto avaliable_samples = get_avaliable_sample_count(device);
				auto found = std::find(avaliable_samples.begin(), avaliable_samples.end(), sample_count);
				return found != avaliable_samples.end();
			}

			inline vk::SurfaceFormatKHR choice_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats)
			{
				if (available_formats.size() == 1 && available_formats[0].format == vk::Format::eUndefined)
					return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

				for (const auto& available_format : available_formats)
				{
					if (available_format.format == vk::Format::eB8G8R8A8Unorm && available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
						return available_format;
				}

				return available_formats[0];
			}

			inline vk::PresentModeKHR choice_surface_present_mode(const std::vector<vk::PresentModeKHR>& present_modes)
			{
				vk::PresentModeKHR best_mode = vk::PresentModeKHR::eFifo;

				for (const auto& present_mode : present_modes)
				{
					if (present_mode == vk::PresentModeKHR::eMailbox)
						return present_mode;
					else if (present_mode == vk::PresentModeKHR::eImmediate)
						best_mode = present_mode;
				}

				return best_mode;
			}

			inline vk::Extent2D choice_swapchain_extent(const vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
			{
				if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
					return capabilities.currentExtent;

				vk::Extent2D actual_extent = { width, height };

				actual_extent.width = (std::max)(capabilities.minImageExtent.width, (std::min)(capabilities.maxImageExtent.width, actual_extent.width));
				actual_extent.height = (std::max)(capabilities.minImageExtent.height, (std::min)(capabilities.maxImageExtent.height, actual_extent.height));

				return actual_extent;
			}

			inline vk::DebugUtilsLabelEXT make_debug_label(const char* label, uint32_t color)
			{
				vk::DebugUtilsLabelEXT debug_label{};
				debug_label.pLabelName = label;
				debug_label.color[0] = ((color >> 24) & 0xFF) / 255.0f;
				debug_label.color[1] = ((color >> 16) & 0xFF) / 255.0f;
				debug_label.color[2] = ((color >> 8) & 0xFF) / 255.0f;
				debug_label.color[3] = (color & 0xFF) / 255.0f;
				return debug_label;
			}

			inline uint32_t convert_vk_queue_flags_to_impl(vk::QueueFlags flags)
			{
				uint32_t mask{ 0u };
				if (flags & vk::QueueFlagBits::eGraphics)
					mask |= QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
				if (flags & vk::QueueFlagBits::eCompute)
					mask |= QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT;
				if (flags & vk::QueueFlagBits::eTransfer)
					mask |= QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT;

				return mask;
			}

			inline FRenderApiErrorInfo make_api_error(vk::ObjectType type, vk::Result result, const std::string& error_text = "")
			{
				FRenderApiErrorInfo error_info{};
				error_info.fault = result != vk::Result::eSuccess;

				if (error_info.fault)
					error_info.description = std::format("\"{}\": {}. Reason: {}.", vk::to_string(type), error_text, vk::to_string(result));

				return error_info;
			}

			inline void check_api_result(const FRenderApiErrorInfo& result)
			{
				if (result.fault)
				{
					log_error(result.description);
					throw std::runtime_error(result.description);
				}
			}

			inline vk::DeviceSize get_alligned_instance_size(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment)
			{
				if (minOffsetAlignment > 0ull)
					return (instanceSize + minOffsetAlignment - 1ull) & ~(minOffsetAlignment - 1ull);
				return instanceSize;
			}

			inline vk::PipelineStageFlags2KHR get_translation_stage_flags(vk::ImageLayout layout)
			{
				switch (layout)
				{
				case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits2KHR::eTopOfPipe;
				case vk::ImageLayout::ePreinitialized: return vk::PipelineStageFlagBits2KHR::eHost;
				case vk::ImageLayout::eTransferDstOptimal:
				case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits2KHR::eTransfer;
				case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput;
				case vk::ImageLayout::eDepthAttachmentOptimal: return vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests | vk::PipelineStageFlagBits2KHR::eLateFragmentTests;
				case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return vk::PipelineStageFlagBits2KHR::eFragmentShadingRateAttachmentKHR;
				case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits2KHR::eFragmentShader | vk::PipelineStageFlagBits2KHR::eVertexShader;
				case vk::ImageLayout::ePresentSrcKHR: return vk::PipelineStageFlagBits2KHR::eBottomOfPipe;
				default: return vk::PipelineStageFlagBits2KHR::eNone;
				}
			}

			inline vk::AccessFlags2KHR get_translation_access_flags(vk::ImageLayout layout)
			{
				switch (layout)
				{
				case vk::ImageLayout::eUndefined:
				case vk::ImageLayout::ePresentSrcKHR: return vk::AccessFlagBits2KHR::eNone;
				case vk::ImageLayout::ePreinitialized: return vk::AccessFlagBits2KHR::eHostWrite;
				case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits2KHR::eColorAttachmentRead | vk::AccessFlagBits2KHR::eColorAttachmentWrite;
				case vk::ImageLayout::eDepthAttachmentOptimal: return vk::AccessFlagBits2KHR::eDepthStencilAttachmentRead | vk::AccessFlagBits2KHR::eDepthStencilAttachmentWrite;
				case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return vk::AccessFlagBits2KHR::eFragmentShadingRateAttachmentReadKHR;
				case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits2KHR::eShaderRead | vk::AccessFlagBits2KHR::eInputAttachmentRead;
				case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits2KHR::eTransferRead;
				case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits2KHR::eTransferWrite;
				default: return vk::AccessFlagBits2KHR::eNone;
				}
			}

			inline vk::ImageLayout get_render_target_final_layout(const FVkTexture& texture)
			{
				auto usage_flags = get_texture_usage_flags(texture);

				if (usage_flags & vk::ImageUsageFlagBits::eColorAttachment)
				{
					if (usage_flags & vk::ImageUsageFlagBits::eTransferSrc)
						return vk::ImageLayout::eTransferSrcOptimal;
					if ((usage_flags & vk::ImageUsageFlagBits::eSampled) && (usage_flags & vk::ImageUsageFlagBits::eStorage))
						return vk::ImageLayout::eGeneral;
					else if (usage_flags & vk::ImageUsageFlagBits::eSampled)
						return vk::ImageLayout::eShaderReadOnlyOptimal;
					else if (usage_flags & vk::ImageUsageFlagBits::eInputAttachment)
					{
						if (usage_flags & vk::ImageUsageFlagBits::eSampled)
							return vk::ImageLayout::eShaderReadOnlyOptimal;
						else
							return vk::ImageLayout::eColorAttachmentOptimal;
					}
				}
				else if (usage_flags & vk::ImageUsageFlagBits::eDepthStencilAttachment)
				{
					if (usage_flags & vk::ImageUsageFlagBits::eSampled)
						return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
					else
						return vk::ImageLayout::eDepthStencilAttachmentOptimal;
				}

				return vk::ImageLayout::ePresentSrcKHR;
			}

			inline vk::AttachmentLoadOp get_attachment_load_op(const FVkTexture& texture)
			{
				return vk::AttachmentLoadOp::eClear;
			}

			inline vk::AttachmentStoreOp get_attachment_store_op(const FVkTexture& texture)
			{
				bool transferable = (texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT);
				bool samplable = (texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT);
				bool depth = (texture.initialData.base.typeFlags & TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT);

				if ((transferable || samplable) && !depth)
					return vk::AttachmentStoreOp::eStore;

				return vk::AttachmentStoreOp::eDontCare;
			}

			inline FVkImageTranslationInfo get_texture_layout_translation_info(const FVkTexture& texture, const FVkTextureTranslationInfo& translation_info)
			{
				FVkImageTranslationInfo image_layout_translation{};
				image_layout_translation.srcStageMask = get_translation_stage_flags(translation_info.oldLayout);
				image_layout_translation.srcAccessMask = get_translation_access_flags(translation_info.oldLayout);
				image_layout_translation.oldLayout = translation_info.oldLayout;
				image_layout_translation.dstStageMask = get_translation_stage_flags(translation_info.newLayout);
				image_layout_translation.dstAccessMask = get_translation_access_flags(translation_info.newLayout);
				image_layout_translation.newLayout = translation_info.newLayout;
				image_layout_translation.handle = texture.image.handle;

				image_layout_translation.subresourceRange.aspectMask = get_texture_aspect_flags(texture);
				image_layout_translation.subresourceRange.baseArrayLayer = translation_info.baseArrayLayer;
				image_layout_translation.subresourceRange.layerCount = texture.totalLayers;
				image_layout_translation.subresourceRange.baseMipLevel = translation_info.baseMipLevel;
				image_layout_translation.subresourceRange.levelCount = texture.initialData.base.mipLevels;

				return image_layout_translation;
			}

			inline bool is_depth_only_format(vk::Format format)
			{
				return format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat;
			}
		}
	}
}