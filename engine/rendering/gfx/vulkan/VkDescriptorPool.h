#pragma once

#include "VkDescriptorSet.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			enum class CanAllocateResult
			{
				Success,
				MaxSetsLimit,
				MaxDescriptorLimit,
				InvalidFlags,
				Other
			};

			class DescriptorPool final : public VulkanResource<vk::DescriptorPool>
			{
			public:
				DescriptorPool(Device& device, DescriptorSetLayout& layout, DescriptorPool* previous_pool = nullptr);
				DescriptorPool(const DescriptorPool&) = delete;
				DescriptorPool(DescriptorPool&&);
				~DescriptorPool();

				DescriptorPool& operator=(const DescriptorPool&) = delete;
				DescriptorPool& operator=(DescriptorPool&&) = delete;

				void Reset();
				VulkanDescriptorSet* Allocate(DescriptorSetLayout& layout, const std::string& debug_name = "");
				vk::Result Free(VulkanDescriptorSet* descriptor_set);

				bool IsExpired() const;
				CanAllocateResult CanAllocate(DescriptorSetLayout& layout);
			private:
				Device& device;

				// Initial number of descriptor count
				PoolSizes initial_sizes{};

				// Avaliable number of descriptors
				PoolSizes avaliable_sizes{};
				std::vector<VulkanDescriptorSet*> allocated_descriptors;

				CanAllocateResult last_allocation_result{};

				uint32_t allocated_count{ 0u };
				int32_t countdown_count{ 0 };
			};
		}
	}
}