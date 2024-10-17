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
			class Texture;
			class Buffer;

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
				std::span<uint32_t> shader_code{};
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
				Texture* texture_handle{ nullptr };
				uint64_t upload_index{ ~0ull };
				std::vector<std::vector<uint64_t>> mip_layer_offsets{};
			};

			struct VkBufferUploadInfo
			{
				Buffer* buffer_handle{ nullptr };
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

			struct FVkTimestampQuery
			{
				vk::QueryPool handle{};
				utl::vector<u64> values{};
				u32 queryIndex{ 0ull };
			};

			struct VkBufferUpdateInfo
			{
				Buffer* dst_buffer{ nullptr };
				vk::DeviceSize update_size{ 0ull };

				std::vector<vk::BufferCopy2KHR> copy_regions{};

				struct
				{
					uint8_t* data{ nullptr };
					vk::DeviceSize avaliable_size{ 0ull };
					size_t buffer_index{ ~0ull };
				} map_info;
			};
		}
	}
}

#define RETURN_WHEN_FAULT(result) \
if(result.fault) \
return result;