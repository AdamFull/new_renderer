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
				vk::SampleCountFlagBits sampleCount{ vk::SampleCountFlagBits::e1 };
				uint32_t patchControlPoints{ 0u };

				std::vector<vk::Format> colorAttachmentFormats{};
				vk::Format depthAttachmentFormat{ vk::Format::eUndefined };

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


			// ---------------------------------------------------------------------------------
			// ====================================| BUFFER |===================================
			// ---------------------------------------------------------------------------------
			struct FVkBuffer
			{
				vk::Buffer handle{};
				vma::Allocation memory{};
				vk::DeviceSize size{};
				vk::BufferUsageFlags usageFlags{};
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

			struct FVkDescriptorBufferResourceInfo
			{
				vk::DeviceAddress address{};
				vk::DeviceSize size{};
				vk::Format format{};
			};

			struct FVkDescriptorTextureResourceInfo
			{
				vk::ImageView handle{};
				vk::Sampler sampler{};
				vk::ImageLayout layout{};
			};

			struct FVkDescriptorResourceInfo
			{
				uint32_t layout{ 0u };
				FVkDescriptorBufferResourceInfo bufferInfo{};
				FVkDescriptorTextureResourceInfo textureInfo{};
				vk::DescriptorType descriptorType{};
			};

			struct FVkDescriptorSetUpdateInfo
			{
				uint32_t set{ 0u };
				std::vector<FVkDescriptorResourceInfo> resourceInfos{};
			};

			struct FVkDescriptorBufferUpdateInfo
			{
				std::vector<FVkDescriptorSetUpdateInfo> updateInfos{};
			};

			// HIGH LEVEL OBJECTS

			struct FVkShader
			{
				std::vector<vk::Pipeline> handles{};
				vk::PipelineLayout layout{};
				std::vector<vk::ShaderModule> shaderModules{};
				std::unordered_map<uint32_t, vk::DescriptorSetLayout> descriptorSetLayouts{};
				std::unordered_map<uint32_t, FVkBuffer> descriptorBuffers{};
				std::vector<vk::PushConstantRange> pushConstantRanges{};
			};

			struct FVkGraphicsShaderCreateInfo
			{
				vk::PrimitiveTopology primitiveTopology{};
				vk::CullModeFlags cullMode{};
				vk::FrontFace frontFace{};
				bool depthTest{ false };
				vk::SampleCountFlagBits sampleCount{ vk::SampleCountFlagBits::e1 };
				uint32_t patchControlPoints{ 0u };

				std::vector<vk::Format> colorAttachmentFormats{};

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
			enum EVkTextureImageTypeFlagBits
			{
				TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT = 1 << 0,
				TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT = 1 << 1,
				TEXTURE_IMAGE_TYPE_2D_FLAG_BIT = 1 << 2,
				TEXTURE_IMAGE_TYPE_3D_FLAG_BIT = 1 << 3
			};

			struct FVkTextureInitialData
			{
				vk::ImageLayout layout{};
				vk::Format format{};
				vk::ImageAspectFlags aspectFlags{};
				vk::ImageUsageFlags usageFlags{};
				vk::SampleCountFlagBits sampleCount{};
				vk::Extent3D extent{};

				uint32_t imageTypeFlags{ 0u };
				uint32_t mipLevelCount{ 1u };
				uint32_t arrayLayersCount{ 0u };
				uint32_t numDimensions{ 0u };
			};

			struct FVkTexture
			{
				FVkImage image{};
				std::vector<vk::ImageView> imageViews{};
				vk::Sampler sampler{};

				FVkTextureInitialData initialData;

				uint32_t totalLayers{ 1u };

				bool isRenderTarget{ false };
			};

			struct FVkTextureCreateInfo
			{
				vk::Format format{};
				vk::ImageUsageFlags usageFlags{};
				vk::ImageAspectFlags aspectFlags{ vk::ImageAspectFlagBits::eColor };
				vk::Extent3D extent{};
				vk::ImageLayout layout{};

				uint32_t imageTypeFlags{ 0u };

				uint32_t mipLevelCount{ 1u };
				uint32_t arrayLayersCount{ 0u };
				uint32_t numDimensions{ 0u };

				vk::SampleCountFlagBits sampleCount{ };

				bool createSampler{ true };
				vk::Filter filter{ vk::Filter::eLinear };
				vk::SamplerAddressMode addressMode{};
				bool enableAnisotropy{ false };
				bool enableCompareOp{ false };

#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkRenderingInfo
			{
				vk::Rect2D renderArea{};
				vk::Rect2D scissorRect{};
				bool flipViewport{ false };

				std::vector<size_t> colorAttachments{};
				size_t depthAttachment{ ~0ull };
			};

			struct FVkImageTranslationInfo
			{
				vk::Image handle;

				vk::PipelineStageFlags2KHR srcStageMask{};
				vk::AccessFlags2KHR srcAccessMask{};
				vk::ImageLayout oldLayout{};
				uint32_t srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

				vk::PipelineStageFlags2KHR dstStageMask{};
				vk::AccessFlags2KHR dstAccessMask{};
				vk::ImageLayout newLayout{};
				uint32_t dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

				vk::ImageSubresourceRange subresourceRange{};
			};

			struct FVkBufferTranslationInfo
			{
				vk::Buffer handle{};

				vk::PipelineStageFlags2KHR srcStageMask{};
				vk::AccessFlags2KHR srcAccessMask{};
				uint32_t srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

				vk::PipelineStageFlags2KHR dstStageMask{};
				vk::AccessFlags2KHR dstAccessMask{};
				uint32_t dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

				vk::DeviceSize size{};
				vk::DeviceSize offset{};
			};

			struct FVkMemoryTranslationInfo
			{
				vk::PipelineStageFlags2KHR srcStageMask{};
				vk::AccessFlags2KHR srcAccessMask{};

				vk::PipelineStageFlags2KHR dstStageMask{};
				vk::AccessFlags2KHR dstAccessMask{};
			};

			struct FVkTranslationInfo
			{
				std::vector<FVkImageTranslationInfo> imageTranslationInfos{};
				std::vector<FVkBufferTranslationInfo> bufferTranslationInfos{};
				std::vector<FVkMemoryTranslationInfo> memoryTranslationInfos{};
			};

			struct FVkTextureTranslationInfo
			{
				vk::ImageLayout oldLayout{};
				vk::ImageLayout newLayout{};
				uint32_t baseArrayLayer{ 0u };
				uint32_t baseMipLevel{ 0u };
			};
		}
	}
}

#define RETURN_WHEN_FAULT(result) \
if(result.fault) \
return result;