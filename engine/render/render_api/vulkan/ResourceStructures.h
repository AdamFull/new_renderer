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

			constexpr uint32_t k_max_descriptor_sets{ 8u };
			constexpr uint32_t k_max_push_constant_range_count{ 16u };

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

			// HIGH LEVEL OBJECTS
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