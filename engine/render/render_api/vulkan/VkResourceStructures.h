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
			class DescriptorAllocator;
			class DescriptorSetLayout;
			class DescriptorSet;

			constexpr size_t k_max_descriptor_sets{ 8ull };

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
				std::array<vk::DescriptorSetLayout, k_max_descriptor_sets> descriptor_set_layouts{};
				size_t descriptor_set_layout_count{ 0ull };

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
				std::unique_ptr<DescriptorAllocator> descriptor_allocator{};
				std::unique_ptr<DescriptorSetLayout> descriptor_set_layout{};
				std::array<std::unique_ptr<DescriptorSet>, 8ull> descriptor_sets{};
				utl::hash_map<u64, u64> uniform_buffers{};
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