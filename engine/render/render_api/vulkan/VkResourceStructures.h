#pragma once

#include <vma/vk_mem_alloc.hpp>

#include <logger/logger.h>

#include <interface/render/RenderInterface.h>

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

			struct FVkThreadKeyResolver
			{
				FVkThreadKeyResolver() : threadId{ std::this_thread::get_id() }, queueType{ QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT } {}
				FVkThreadKeyResolver(uint32_t queue_type, std::thread::id thread_id = std::this_thread::get_id()) : threadId{ thread_id }, queueType{ queue_type } {}

				friend bool operator==(const FVkThreadKeyResolver& lhs, const FVkThreadKeyResolver& rhs)
				{
					return lhs.threadId == rhs.threadId && lhs.queueType == rhs.queueType;
				}

				std::thread::id threadId{};
				uint32_t queueType{ 0 };
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
			struct FVkCommandPool
			{
				FVkCommandPool() { incrementUsage(); }
				~FVkCommandPool() { decrementUsage(); }

				FVkCommandPool(const FVkCommandPool& other) : handle(other.handle), threadId(other.threadId)
				{
					incrementUsage();
				}

				FVkCommandPool& operator=(const FVkCommandPool& other)
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
				std::thread::id threadId{ std::this_thread::get_id() };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ===============================| COMMAND BUFFER |================================
			// ---------------------------------------------------------------------------------
			struct FVkCommandBuffer
			{
				std::vector<vk::CommandBuffer> handles{};
				std::thread::id threadId{};
				uint32_t mask{ 0u };
				bool isRunning{ false };
				uint32_t commandBufferIndex{ 0u };

				const vk::CommandBuffer& get() const
				{
					return handles.at(commandBufferIndex);
				}
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
				vk::DescriptorSetLayoutCreateFlags flags{};
				void* next{ nullptr };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkDescriptorPool
			{
				vk::DescriptorPool handle{};
				std::thread::id thread_id{};
			};

			struct FVkDescriptorPoolCreateInfo
			{
				std::vector<vk::DescriptorPoolSize> poolSizes{};
				uint32_t usageCount{ 1u };
				vk::DescriptorPoolCreateFlags flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkDescriptorSet
			{
				// Per frame descriptor sets
				std::vector<vk::DescriptorSet> handles{};
			};

			struct FVkDescriptorSetCreateInfo
			{
				vk::DescriptorPool pool{};
				vk::DescriptorSetLayout layout{};
				uint32_t descriptorCount{ 0u };
				void* next{ nullptr };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkGetCommandBufferInfo
			{
				uint32_t queueType{};
				std::thread::id threadId{ std::this_thread::get_id() };
				size_t bufferIndex{};
			};

			struct FVkBindDescriptorInfo
			{
				vk::PipelineLayout layout{};
				vk::PipelineBindPoint bindPoint{};
			};

			// ---------------------------------------------------------------------------------
			// ====================================| PIPELINE |=================================
			// ---------------------------------------------------------------------------------
			struct FVkShaderModuleCreateInfo
			{
				std::vector<uint32_t>* shaderCode{};
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
				vk::SampleCountFlagBits sampleCount{ vk::SampleCountFlagBits::e1 };
				uint32_t patchControlPoints{ 0u };

				std::vector<vk::Format> colorAttachmentFormats{};
				vk::Format depthAttachmentFormat{ vk::Format::eUndefined };
				vk::Format stencilAttachmentFormat{ vk::Format::eUndefined };

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
				vk::DeviceSize instanceCount{};
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

			// HIGH LEVEL OBJECTS

			struct FVkShader
			{
				std::vector<vk::Pipeline> handles{};
				vk::PipelineLayout layout{};
				std::vector<vk::ShaderModule> shaderModules{};
				std::unordered_map<uint32_t, vk::DescriptorSetLayout> descriptorSetLayouts{};
				std::unordered_map<uint32_t, FVkDescriptorSet> descriptorSets{};
				std::unordered_map<uint32_t, uint32_t> descriptorUsage{};
				FVkDescriptorPool descriptorPool{};
				std::vector<vk::PushConstantRange> pushConstantRanges{};

				std::unordered_map<size_t, size_t> uniformBuffers{};
				size_t maximumDescriptorSlots{ 0ull };
				
				vk::PipelineBindPoint bindPoint{};

				size_t reflection{ ~0ull };
			};

			struct FVkShaderResourcesInfo
			{
				uint32_t usageCount{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};


			// ---------------------------------------------------------------------------------
			// ===================================| TEXTURE |===================================
			// ---------------------------------------------------------------------------------
			struct FVkTextureInitialData
			{
				FTextureCreateInfo base{};
				vk::SampleCountFlagBits sampleCount{ vk::SampleCountFlagBits::e1 };
			};

			struct FVkTexture
			{
				FVkImage image{};
				std::vector<vk::ImageView> imageViews{};
				vk::Sampler sampler{};
				vk::ImageLayout layout{};
				uint32_t totalLayers{ 1u };
				bool isRenderTarget{ false };

				FVkTextureInitialData initialData;
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkTextureCreateInfo
			{
				FTextureCreateInfo base{};

				vk::SampleCountFlagBits sampleCount{ vk::SampleCountFlagBits::e1 };

				bool createSampler{ true };
				vk::Filter filter{ vk::Filter::eLinear };
				vk::SamplerAddressMode addressMode{};
				bool enableAnisotropy{ false };
				bool enableCompareOp{ false };

#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
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

			struct FVkGlobalDescriptor
			{
				FVkDescriptorPool descriptorPool{};
				vk::DescriptorSetLayout deecriptorSetLayout{};
				FVkDescriptorSet descriptorSet{};
			};
		}
	}
}

namespace std 
{
	template<>
	struct hash<engine::render::vulkan::FVkThreadKeyResolver>
	{
		size_t operator()(const engine::render::vulkan::FVkThreadKeyResolver& key) const
		{
			size_t hash1 = hash<thread::id>()(key.threadId);
			size_t hash2 = hash<uint32_t>()(key.queueType);
			return hash1 ^ (hash2 << 1);
		}
	};
}

#define RETURN_WHEN_FAULT(result) \
if(result.fault) \
return result;