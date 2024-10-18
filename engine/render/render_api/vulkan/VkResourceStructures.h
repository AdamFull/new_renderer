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
				static constexpr size_t k_max_descriptor_sets{ 100ull };
				std::array<vk::DescriptorSet, k_max_descriptor_sets> handles{};
				size_t descriptor_count{ 0ull };
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
				static constexpr size_t k_max_pipelines_handles{ 4ull };
				static constexpr size_t k_max_shader_module_handles{ 8ull };

				std::array<vk::Pipeline, k_max_pipelines_handles> handles{};
				size_t pipeline_count{ 0ull };

				std::array<vk::ShaderModule, k_max_shader_module_handles> shaderModules{};
				size_t shader_module_count{ 0ull };

				vk::PipelineLayout layout{};
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
				static constexpr size_t k_max_translations{ 64ull };

				std::array<vk::ImageMemoryBarrier2KHR, k_max_translations> imageTranslationInfos{};
				size_t image_tanslation_count{ 0ull };

				std::array<vk::BufferMemoryBarrier2KHR, k_max_translations> bufferTranslationInfos{};
				size_t buffer_tanslation_count{ 0ull };

				std::array<vk::MemoryBarrier2KHR, k_max_translations> memoryTranslationInfos{};
				size_t memory_tanslation_count{ 0ull };
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

			struct VkTextureUpdateInfo
			{
				Texture* dst_texture{ nullptr };
				vk::DeviceSize update_size{ 0ull };

				struct
				{
					uint8_t* data{ nullptr };
					vk::DeviceSize avaliable_size{ 0ull };
					size_t buffer_index{ ~0ull };
				} map_info;
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