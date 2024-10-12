#pragma once
#include <thread>

#include <vma/vk_mem_alloc.hpp>

#include <util/Logger.h>
#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Image;
			class ImageView;

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

			// =================================================================================
			// ==================================Object Handles=================================
			// =================================================================================

			struct FSwapchain
			{
				vk::SwapchainKHR handle{ nullptr };
				vk::Format format{};
				vk::Extent2D extent{};
				utl::vector<vk::Image> images{};
				utl::vector<vk::ImageView> imageViews{};
				utl::vector<vk::Semaphore> imageAvaliableSemaphores{};
				utl::vector<vk::Semaphore> renderFinishedSemaphores{};
				utl::vector<vk::Fence> inFlightFences{};
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
				std::thread::id threadId{ std::this_thread::get_id() };
#endif
			};

			// ---------------------------------------------------------------------------------
			// ===============================| COMMAND BUFFER |================================
			// ---------------------------------------------------------------------------------
			struct FVkCommandBuffer
			{
				utl::vector<vk::CommandBuffer> handles{};
				EQueueTypeFlagBits mask{};
				bool isRunning{ false };
				u32 commandBufferIndex{ 0u };

				vk::CommandBuffer& get() { return handles[commandBufferIndex]; }
				const vk::CommandBuffer& get() const { return handles[commandBufferIndex]; }
			};

			struct FVkCommandBufferCreateInfo
			{
				EQueueTypeFlagBits mask{};
				vk::CommandBufferLevel level{};
				u32 count{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkDescriptorSetLayoutCreateInfo
			{
				utl::vector<vk::DescriptorSetLayoutBinding> layoutBindings{};
				vk::DescriptorSetLayoutCreateFlags flags{};
				void* next{ nullptr };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkDescriptorPool
			{
				vk::DescriptorPool handle{};
			};

			struct FVkDescriptorPoolCreateInfo
			{
				utl::vector<vk::DescriptorPoolSize> poolSizes{};
				u32 usageCount{ 1u };
				vk::DescriptorPoolCreateFlags flags{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkDescriptorSet
			{
				// Per frame descriptor sets
				utl::vector<vk::DescriptorSet> handles{};
			};

			struct FVkDescriptorSetCreateInfo
			{
				vk::DescriptorPool pool{};
				vk::DescriptorSetLayout layout{};
				u32 descriptorCount{ 0u };
				void* next{ nullptr };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkGetCommandBufferInfo
			{
				u32 queueType{};
				std::thread::id threadId{ std::this_thread::get_id() };
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
				utl::vector<u32>* shaderCode{};
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct FVkPipelineLayoutCreateInfo
			{
				utl::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};
				utl::vector<vk::PushConstantRange> pushConstantRanges{};
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
				u32 patchControlPoints{ 0u };

				bool enableAlphaBlend{ false };

				utl::vector<vk::Format> colorAttachmentFormats{};
				vk::Format depthAttachmentFormat{ vk::Format::eUndefined };
				vk::Format stencilAttachmentFormat{ vk::Format::eUndefined };

				vk::CompareOp depthCompareOp{ vk::CompareOp::eLessOrEqual };

				utl::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions{};
				utl::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions{};

				utl::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
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

			struct FVkRayTracingPipelineCreateInfo
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
//			struct FVkBuffer
//			{
//				vk::Buffer handle{};
//				vma::Allocation memory{};
//				vk::DeviceSize size{};
//				vk::DeviceSize instanceCount{};
//				vk::BufferUsageFlags usageFlags{};
//				uint64_t address{};
//
//				std::string name{};
//
//				bool isMapped{ false };
//				bool isAlive{ false };
//			};
//
//			struct FVkBufferCreateInfo
//			{
//				vk::DeviceSize instanceSize{ 0ull };
//				vk::DeviceSize instanceCount{ 0ull };
//				vk::DeviceSize minOffsetAlignment{ 1ull };
//				vk::BufferUsageFlags usageFlags{};
//				vma::MemoryUsage memoryUsage{};
//#ifndef BUILD_WITHOUT_DEBUGGIND
//				std::string name{};
//#endif
//			};

			// HIGH LEVEL OBJECTS

			struct FVkShader
			{
				utl::vector<vk::Pipeline> handles{};
				vk::PipelineLayout layout{};
				utl::vector<vk::ShaderModule> shaderModules{};
				utl::hash_map<u32, vk::DescriptorSetLayout> descriptorSetLayouts{};
				utl::hash_map<u32, FVkDescriptorSet> descriptorSets{};
				utl::hash_map<u32, u32> descriptorUsage{};
				FVkDescriptorPool descriptorPool{};
				utl::vector<vk::PushConstantRange> pushConstantRanges{};

				utl::hash_map<u64, u64> uniformBuffers{};
				u64 maximumDescriptorSlots{ 0ull };
				
				vk::PipelineBindPoint bindPoint{};

				u64 reflection{ INVALID_HANDLE };

				bool isAlive{ false };
			};

			struct FVkShaderResourcesInfo
			{
				u32 usageCount{ 1u };
#ifndef BUILD_WITHOUT_DEBUGGIND
				std::string name{};
#endif
			};

			struct VkTextureUploadInfo
			{
				FAPIHandle texture_id{ INVALID_HANDLE };
				uint64_t upload_index{ ~0ull };
				std::vector<std::vector<uint64_t>> mip_layer_offsets{};
			};

			struct VkBufferUploadInfo
			{
				FAPIHandle buffer_id{};
				uint64_t upload_index{ ~0ull };
				uint64_t upload_size{ 0ull };
				std::vector<FBufferUploadRegion> copy_regions{};
			};

			struct FVkTranslationInfo
			{
				utl::vector<vk::ImageMemoryBarrier2KHR> imageTranslationInfos{};
				utl::vector<vk::BufferMemoryBarrier2KHR> bufferTranslationInfos{};
				utl::vector<vk::MemoryBarrier2KHR> memoryTranslationInfos{};
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
				u64 textureId{ INVALID_HANDLE };
			};

			struct FVkGlobalDescriptor
			{
				FVkDescriptorPool descriptorPool{};
				vk::DescriptorSetLayout deecriptorSetLayout{};
				FVkDescriptorSet descriptorSet{};
				utl::hash_map<u64, u64> uniformBuffers{};
			};

			struct FVkPerQueueResources
			{
				FVkCommandPool commandPool{};
				utl::vector<FVkCommandBuffer> commandBuffers{};
			};

			struct FVkTimestampQuery
			{
				vk::QueryPool handle{};
				utl::vector<u64> values{};
				u32 queryIndex{ 0ull };
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

				i32 use_count() { return refCounter.load(std::memory_order_relaxed); }

				utl::hash_map<EQueueTypeFlagBits, FVkPerQueueResources> perQueue{};
				FVkDescriptorPool descriptorPool{};
				utl::hash_map<u64, FVkDescriptorSet> descriptorSets{};
			private:
				void incrementUsage() { refCounter.fetch_add(1, std::memory_order_relaxed); }
				void decrementUsage() { refCounter.fetch_sub(1, std::memory_order_relaxed); }
				std::atomic<i32> refCounter{ 0 };
			};

			struct FVkNativeContext
			{
				vk::Instance instance{};
				vk::PhysicalDevice physicalDevice{};
				vk::Device device{};

				uint32_t presentQueueFamily{};
				vk::Queue presentQueue{};

				vk::PipelineCache pipelineCache{};
				vk::Format backbufferFormat{};
				uint32_t backbufferFrames{};
				vk::SampleCountFlagBits samples{};

				const vk::AllocationCallbacks* allocationCallbacks{};

				vma::Allocator memoryAllocator{};

			};
		}
	}
}

#define RETURN_WHEN_FAULT(result) \
if(result.fault) \
return result;