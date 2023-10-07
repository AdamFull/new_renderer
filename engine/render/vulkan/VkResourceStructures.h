#pragma once

#include <vma/vk_mem_alloc.hpp>

#include <logger/logger.h>

#include <stdexcept>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <atomic>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			struct FRenderApiErrorInfo
			{
				bool fault{ false };
				std::string description{};
			};

			enum EnableValidationFeatureFlagBits
			{
				GPU_ASSISTED_FLAG_BIT = 1 << 0,
				GPU_ASSISTED_RESERVE_BINDING_SLOT_FLAG_BIT = 1 << 1,
				BEST_PRACTICE_FLAG_BIT = 1 << 2,
				DEBUG_PRINTF_FLAG_BIT = 1 << 3,
				SYNCHRONIZATION_VALIDATION_FLAG_BIT = 1 << 4
			};

			enum EQueueFamilyUsageFlagBits
			{
				QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT = 1 << 0,
				QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT = 1 << 1,
				QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT = 1 << 2,
				QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT = 1 << 3
			};

			struct FQueueFamilies
			{
				std::vector<uint32_t> uniqueFamilies{};
				std::unordered_map<uint32_t, uint32_t> familyUsages;
				std::unordered_map<uint32_t, uint32_t> queueToFamily;
				uint32_t queueMask{ 0u };
			};

			enum EPhysicalDeviceCompatibilityFlagBits
			{
				PHYSICAL_DEVICE_COMPAT_BETTER_THEN_NOTHING_FLAG_BIT = 1 << 0,
				PHYSICAL_DEVICE_COMPAT_ASYNC_COMPUTE_FLAG_BIT = 1 << 1,
				PHYSICAL_DEVICE_COMPAT_ASYNC_TRANSFER_FLAG_BIT = 1 << 2,
				PHYSICAL_DEVICE_COMPAT_BEST_CANDIDATE_FLAG_BIT = PHYSICAL_DEVICE_COMPAT_BETTER_THEN_NOTHING_FLAG_BIT |
				PHYSICAL_DEVICE_COMPAT_ASYNC_COMPUTE_FLAG_BIT |
				PHYSICAL_DEVICE_COMPAT_ASYNC_TRANSFER_FLAG_BIT
			};

			// =================================================================================
			// ==================================Object Handles=================================
			// =================================================================================

			struct FVkPhysicalDevice
			{
				vk::PhysicalDevice handle{ nullptr };
				vk::PhysicalDeviceProperties properties{};
				std::vector<std::string> avaliableExtensions{};
				FQueueFamilies queueFamilies{};
				std::unordered_map<uint32_t, bool> supportedExtensions{};
				uint32_t compatFlags{ 0u };
			};

			struct FVkLogicalDevice
			{
				vk::Device handle{ nullptr };
				std::unordered_map<uint32_t, vk::Queue> queues{};
				void* lastExtensionFeature{ nullptr };
				std::map<vk::StructureType, std::shared_ptr<void>> extensionFeatures{};
			};

			struct FSwapchain
			{
				vk::SwapchainKHR handle{ nullptr };
				vk::Format format{};
				vk::Extent2D extent{};
				std::vector<vk::Image> images{};
				std::vector<vk::ImageView> imageViews{};
				std::vector<vk::Semaphore> imageAvaliableSemaphores{};
				std::vector<vk::Semaphore> renderFinishedSemaphores{};
				std::vector<vk::Fence> inFlightFences{};
				uint32_t framesInFlight{ 2u };
				uint32_t currentFrame{ 0u };
			};

			// ---------------------------------------------------------------------------------
			// ================================| COMMAND POOL |=================================
			// ---------------------------------------------------------------------------------
			struct FCommandPool
			{
				FCommandPool() { incrementUsage(); }
				~FCommandPool() { decrementUsage(); }

				FCommandPool(const FCommandPool& other) : handle(other.handle), threadId(other.threadId)
				{
					incrementUsage();
				}

				FCommandPool& operator=(const FCommandPool& other)
				{
					if (this != &other)
					{
						handle = other.handle;
						threadId = other.threadId;
					}
					return *this;
				}

				int32_t use_count() { return refCounter.load(std::memory_order_relaxed); }

				vk::CommandPool handle{ nullptr };
				std::thread::id threadId{};
			private:
				void incrementUsage() { refCounter.fetch_add(1, std::memory_order_relaxed); }
				void decrementUsage() { refCounter.fetch_sub(1, std::memory_order_relaxed); }
				std::atomic<int32_t> refCounter{ 0 };
			};

			struct FVkCommandPoolCreateInfo
			{
				uint32_t mask{ 0u };
				std::thread::id threadId{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ===============================| COMMAND BUFFER |================================
			// ---------------------------------------------------------------------------------
			struct FVkCommandBuffer
			{
				std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>> handles{};
				std::thread::id threadId{};
				uint32_t mask{ 0u };
				bool isRunning{ false };
				uint32_t commandBufferIndex{ 0u };
			};

			struct FVkCommandBufferCreateInfo
			{
				uint32_t mask{ 0u };
				vk::CommandBufferLevel level{};
				uint32_t count{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ====================================| IMAGE |====================================
			// ---------------------------------------------------------------------------------
			struct FVkImage
			{
				vk::Image handle{};
				vma::Allocation memory{};
			};

			struct FVkImageCreateInfo
			{
				vk::ImageCreateInfo ci{};
				vma::MemoryUsage memoryUsage{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ==================================| IMAGE VIEW |=================================
			// ---------------------------------------------------------------------------------
			struct FVkImageViewCreateInfo
			{
				vk::ImageViewCreateInfo ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ====================================| SAMPLER |==================================
			// ---------------------------------------------------------------------------------
			struct FVkSamplerCreateInfo
			{
				vk::Filter magFilter{};
				vk::Filter minFilter{};
				vk::SamplerAddressMode addressMode{};
				bool enableAnisotropy{ false };
				bool enableCompareOp{ false };
				uint32_t mipLevels{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ===================================| SEMAPHORE |=================================
			// ---------------------------------------------------------------------------------
			struct FVkSemaphoreCreateInfo
			{
				vk::SemaphoreCreateInfo ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ====================================| FENCE |===================================
			// ---------------------------------------------------------------------------------
			struct FVkFenceCreateInfo
			{
				vk::FenceCreateInfo ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkDescriptorSetLayoutCreateInfo
			{
				std::vector<vk::DescriptorSetLayoutBinding> layoutBindings{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ====================================| PIPELINE |=================================
			// ---------------------------------------------------------------------------------
			struct FVkShaderModuleCreateInfo
			{
				std::vector<uint32_t> shaderCode{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkPipelineLayoutCreateInfo
			{
				std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};
				std::vector<vk::PushConstantRange> pushConstantRanges{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkGraphicsPipelineCreateInfo
			{
				vk::PipelineLayout layout{};

				vk::PrimitiveTopology primitiveTopology{};
				vk::CullModeFlags cullMode{};
				vk::FrontFace frontFace{};
				bool depthTest{ false };
				uint32_t colorAttachmentCount{ 0u };
				vk::SampleCountFlagBits sampleCount{};
				uint32_t patchControlPoints{ 0u };

				std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions{};
				std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions{};

				std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkComputePipelineCreateInfo
			{
				vk::PipelineLayout layout{};
				vk::PipelineShaderStageCreateInfo shaderStage{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkShaderReflectionData
			{

			};

			struct FVkShader
			{
				std::vector<vk::Pipeline> handles{};
				vk::PipelineLayout layout{};
				std::vector<vk::ShaderModule> shaderModules{};
				std::unordered_map<uint32_t, vk::DescriptorSetLayout> descriptorSetLayouts{};
				std::vector<vk::PushConstantRange> pushConstantRanges{};
			};

			struct FVkGraphicsShaderCreateInfo
			{
				vk::PrimitiveTopology primitiveTopology{};
				vk::CullModeFlags cullMode{};
				vk::FrontFace frontFace{};
				bool depthTest{ false };
				uint32_t colorAttachmentCount{ 0u };
				vk::SampleCountFlagBits sampleCount{};
				uint32_t patchControlPoints{ 0u };

				std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions{};
				std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions{};

				std::unordered_map<vk::ShaderStageFlagBits, std::vector<uint32_t>> shaderStageCode{};

#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkComputeShaderCreateInfo
			{
				std::vector<std::vector<uint32_t>> shaderCode{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};


			// ---------------------------------------------------------------------------------
			// ===================================| TEXTURE |===================================
			// ---------------------------------------------------------------------------------
			struct FVkTexture
			{
				FVkImage image{};
				std::vector<vk::ImageView> imageViews{};
				vk::Sampler sampler{};
				vk::ImageLayout layout{};
			};

			struct FVkTextureCreateInfo
			{
				vk::Format format{};
				vk::ImageUsageFlags usageFlags{};
				vk::ImageAspectFlags aspectFlags{ vk::ImageAspectFlagBits::eColor };
				vk::Extent3D extent{};
				vk::ImageLayout initialLayout{};

				bool isArray{ false };
				bool isCubemap{ false };

				uint32_t mipLevelCount{ 1u };
				uint32_t arrayLayersCount{ 0u };
				uint32_t numDimensions{ 0u };

				vk::SampleCountFlagBits sampleCount{ };

				vk::Filter filter{ vk::Filter::eLinear };
				vk::SamplerAddressMode addressMode{};
				bool enableAnisotropy{ false };
				bool enableCompareOp{ false };

#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ====================================| BUFFER |===================================
			// ---------------------------------------------------------------------------------
			struct FVkBuffer
			{
				vk::Buffer handle{};
				vma::Allocation memory{};
				vk::DeviceSize size{};
				bool isMapped{ false };
			};

			struct FVkBufferCreateInfo
			{
				vk::DeviceSize instanceSize{ 0ull };
				vk::DeviceSize instanceCount{ 0ull };
				vk::DeviceSize minOffsetAlignment{ 1ull };
				vk::BufferUsageFlags usageFlags{};
				vma::MemoryUsage memoryUsage{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ==================================| RENDERING |==================================
			// ---------------------------------------------------------------------------------
			// 
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
		}
	}
}

#define RETURN_WHEN_FAULT(result) \
if(result.fault) \
return result;