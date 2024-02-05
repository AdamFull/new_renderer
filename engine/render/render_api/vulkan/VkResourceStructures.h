#pragma once

#include <vma/vk_mem_alloc.hpp>

#include <logger/logger.h>
#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			struct FRenderApiErrorInfo
			{
				bool fault{ false };
				stl::string description{};
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
				stl::vector<u32> uniqueFamilies{};
				stl::unordered_map<u32, u32> familyUsages;
				stl::unordered_map<EQueueTypeFlagBits, u32> queueToFamily;
				EQueueTypeFlags queueMask{ 0u };
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
				stl::vector<stl::string> avaliableExtensions{};
				FQueueFamilies queueFamilies{};
				stl::unordered_map<u32, bool> supportedExtensions{};
				u32 compatFlags{ 0u };
			};

			struct FVkLogicalDevice
			{
				vk::Device handle{ nullptr };
				stl::unordered_map<EQueueTypeFlagBits, vk::Queue> queues{};
				void* lastExtensionFeature{ nullptr };
				stl::map<vk::StructureType, stl::shared_ptr<void>> extensionFeatures{};
			};

			struct FSwapchain
			{
				vk::SwapchainKHR handle{ nullptr };
				vk::Format format{};
				vk::Extent2D extent{};
				stl::vector<vk::Image> images{};
				stl::vector<vk::ImageView> imageViews{};
				stl::vector<vk::Semaphore> imageAvaliableSemaphores{};
				stl::vector<vk::Semaphore> renderFinishedSemaphores{};
				stl::vector<vk::Fence> inFlightFences{};
				u32 framesInFlight{ 2u };
				u32 currentFrame{ 0u };
			};

			// ---------------------------------------------------------------------------------
			// ================================| COMMAND POOL |=================================
			// ---------------------------------------------------------------------------------
			struct FVkCommandPool
			{
				vk::CommandPool handle{ nullptr };
			};

			struct FVkCommandPoolCreateInfo
			{
				EQueueTypeFlagBits mask{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::thread::id threadId{ stl::this_thread::get_id() };
#endif
			};

			// ---------------------------------------------------------------------------------
			// ===============================| COMMAND BUFFER |================================
			// ---------------------------------------------------------------------------------
			struct FVkCommandBuffer
			{
				stl::vector<vk::CommandBuffer> handles{};
				EQueueTypeFlagBits mask{};
				bool isRunning{ false };
				u32 commandBufferIndex{ 0u };

				vk::CommandBuffer& get() { return handles.at(commandBufferIndex); }
				const vk::CommandBuffer& get() const { return handles.at(commandBufferIndex); }
			};

			struct FVkCommandBufferCreateInfo
			{
				EQueueTypeFlagBits mask{};
				vk::CommandBufferLevel level{};
				u32 count{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
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
				stl::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ==================================| IMAGE VIEW |=================================
			// ---------------------------------------------------------------------------------
			struct FVkImageViewCreateInfo
			{
				vk::ImageViewCreateInfo ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
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
				u32 mipLevels{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ===================================| SEMAPHORE |=================================
			// ---------------------------------------------------------------------------------
			struct FVkSemaphoreCreateInfo
			{
				vk::SemaphoreCreateInfo ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			// ---------------------------------------------------------------------------------
			// ====================================| FENCE |===================================
			// ---------------------------------------------------------------------------------
			struct FVkFenceCreateInfo
			{
				vk::FenceCreateInfo ci{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkDescriptorSetLayoutCreateInfo
			{
				stl::vector<vk::DescriptorSetLayoutBinding> layoutBindings{};
				vk::DescriptorSetLayoutCreateFlags flags{};
				void* next{ nullptr };
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkDescriptorPool
			{
				vk::DescriptorPool handle{};
			};

			struct FVkDescriptorPoolCreateInfo
			{
				stl::vector<vk::DescriptorPoolSize> poolSizes{};
				u32 usageCount{ 1u };
				vk::DescriptorPoolCreateFlags flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet };
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkDescriptorSet
			{
				// Per frame descriptor sets
				stl::vector<vk::DescriptorSet> handles{};
			};

			struct FVkDescriptorSetCreateInfo
			{
				vk::DescriptorPool pool{};
				vk::DescriptorSetLayout layout{};
				u32 descriptorCount{ 0u };
				void* next{ nullptr };
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkGetCommandBufferInfo
			{
				u32 queueType{};
				stl::thread::id threadId{ stl::this_thread::get_id() };
				u64 bufferIndex{};
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
				stl::vector<u32>* shaderCode{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkPipelineLayoutCreateInfo
			{
				stl::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};
				stl::vector<vk::PushConstantRange> pushConstantRanges{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkGraphicsPipelineCreateInfo
			{
				vk::PipelineLayout layout{};

				vk::PrimitiveTopology primitiveTopology{};
				vk::CullModeFlags cullMode{};
				vk::FrontFace frontFace{};
				vk::SampleCountFlagBits sampleCount{ vk::SampleCountFlagBits::e1 };
				u32 patchControlPoints{ 0u };

				stl::vector<vk::Format> colorAttachmentFormats{};
				vk::Format depthAttachmentFormat{ vk::Format::eUndefined };
				vk::Format stencilAttachmentFormat{ vk::Format::eUndefined };

				stl::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions{};
				stl::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions{};

				stl::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkComputePipelineCreateInfo
			{
				vk::PipelineLayout layout{};
				vk::PipelineShaderStageCreateInfo shaderStage{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
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
				uint64_t address{};

				bool isMapped{ false };
				bool isAlive{ false };
			};

			struct FVkBufferCreateInfo
			{
				vk::DeviceSize instanceSize{ 0ull };
				vk::DeviceSize instanceCount{ 0ull };
				vk::DeviceSize minOffsetAlignment{ 1ull };
				vk::BufferUsageFlags usageFlags{};
				vma::MemoryUsage memoryUsage{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			// HIGH LEVEL OBJECTS

			struct FVkShader
			{
				stl::vector<vk::Pipeline> handles{};
				vk::PipelineLayout layout{};
				stl::vector<vk::ShaderModule> shaderModules{};
				stl::unordered_map<u32, vk::DescriptorSetLayout> descriptorSetLayouts{};
				stl::unordered_map<u32, FVkDescriptorSet> descriptorSets{};
				stl::unordered_map<u32, u32> descriptorUsage{};
				FVkDescriptorPool descriptorPool{};
				stl::vector<vk::PushConstantRange> pushConstantRanges{};

				stl::unordered_map<u64, u64> uniformBuffers{};
				u64 maximumDescriptorSlots{ 0ull };
				
				vk::PipelineBindPoint bindPoint{};

				u64 reflection{ ~0ull };

				bool isAlive{ false };
			};

			struct FVkShaderResourcesInfo
			{
				u32 usageCount{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
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

			struct FVkTextureMipLevel
			{
				vk::ImageView imageView{};
				u32 baseMipLevel{};
				u32 mipLevelCount{ 1u };
			};

			struct FVkTextureLayer
			{
				stl::vector<FVkTextureMipLevel> mips{};
				u32 baseLayer{};
				u32 layerCount{ 1u };
			};

			struct FVkTexture
			{
				FVkImage image{};
				stl::vector<FVkTextureLayer> layers{};
				//stl::vector<vk::ImageView> imageViews{};
				vk::Sampler sampler{};
				vk::ImageLayout layout{};
				u32 totalLayers{ 1u };
				bool isRenderTarget{ false };

				bool isAlive{ false };

				FVkTextureInitialData initialData;
#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkTextureCreateInfo
			{
				FTextureCreateInfo base{};

				vk::SampleCountFlagBits sampleCount{ vk::SampleCountFlagBits::e1 };

				bool createSampler{ true };
				vk::Filter filter{ vk::Filter::eLinear };
				vk::SamplerAddressMode addressMode{ vk::SamplerAddressMode::eClampToEdge };
				bool enableAnisotropy{ false };

#ifndef BUILD_WITHOUT_DEBUGGIND
				stl::string name{};
#endif
			};

			struct FVkImageTranslationInfo
			{
				vk::Image handle;

				vk::PipelineStageFlags2KHR srcStageMask{};
				vk::AccessFlags2KHR srcAccessMask{};
				vk::ImageLayout oldLayout{};
				u32 srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

				vk::PipelineStageFlags2KHR dstStageMask{};
				vk::AccessFlags2KHR dstAccessMask{};
				vk::ImageLayout newLayout{};
				u32 dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

				vk::ImageSubresourceRange subresourceRange{};
			};

			struct FVkBufferTranslationInfo
			{
				vk::Buffer handle{};

				vk::PipelineStageFlags2KHR srcStageMask{};
				vk::AccessFlags2KHR srcAccessMask{};
				u32 srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

				vk::PipelineStageFlags2KHR dstStageMask{};
				vk::AccessFlags2KHR dstAccessMask{};
				u32 dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };

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
				stl::vector<FVkImageTranslationInfo> imageTranslationInfos{};
				stl::vector<FVkBufferTranslationInfo> bufferTranslationInfos{};
				stl::vector<FVkMemoryTranslationInfo> memoryTranslationInfos{};
			};

			struct FVkTextureTranslationInfo
			{
				vk::ImageLayout oldLayout{};
				vk::ImageLayout newLayout{};
				u32 baseArrayLayer{ 0u };
				u32 baseMipLevel{ 0u };
			};

			struct FVkTextureLayoutChangeInfo
			{
				u64 textureId{ ~0ull };
			};

			struct FVkGlobalDescriptor
			{
				FVkDescriptorPool descriptorPool{};
				vk::DescriptorSetLayout deecriptorSetLayout{};
				FVkDescriptorSet descriptorSet{};
				stl::unordered_map<u64, u64> uniformBuffers{};
			};

			struct FVkPerQueueResources
			{
				FVkCommandPool commandPool{};
				stl::vector<FVkCommandBuffer> commandBuffers{};
			};

			struct FVkThreadContext
			{
				FVkThreadContext() { incrementUsage(); }
				~FVkThreadContext() { decrementUsage(); }

				FVkThreadContext(const FVkThreadContext& other) : perQueue(other.perQueue), descriptorPool(other.descriptorPool), descriptorSets(other.descriptorSets)
				{ incrementUsage(); }

				FVkThreadContext& operator=(const FVkThreadContext& other)
				{
					if (this != &other)
					{
						perQueue = other.perQueue;
						descriptorPool.handle = other.descriptorPool.handle;
						descriptorSets = other.descriptorSets;
					}
					return *this;
				}

				i32 use_count() { return refCounter.load(stl::memory_order_relaxed); }

				stl::unordered_map<EQueueTypeFlagBits, FVkPerQueueResources> perQueue{};
				FVkDescriptorPool descriptorPool{};
				stl::unordered_map<u64, FVkDescriptorSet> descriptorSets{};
			private:
				void incrementUsage() { refCounter.fetch_add(1, stl::memory_order_relaxed); }
				void decrementUsage() { refCounter.fetch_sub(1, stl::memory_order_relaxed); }
				stl::atomic<i32> refCounter{ 0 };
			};
		}
	}
}

#define RETURN_WHEN_FAULT(result) \
if(result.fault) \
return result;