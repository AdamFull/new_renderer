#pragma once

#include <base/DescriptorSet.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;

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
				DescriptorPool(RenderDevice& render_device, DescriptorSetLayout& layout, DescriptorPool* previous_pool = nullptr);
				DescriptorPool(const DescriptorPool&) = delete;
				DescriptorPool(DescriptorPool&&);
				~DescriptorPool();

				DescriptorPool& operator=(const DescriptorPool&) = delete;
				DescriptorPool& operator=(DescriptorPool&&) = delete;

				void Reset();
				DescriptorSet& Allocate(DescriptorSetLayout& layout, const std::string& debug_name = "");
				vk::Result Free(DescriptorSet& descriptor_set);

				bool IsExpired() const;
				CanAllocateResult CanAllocate(DescriptorSetLayout& layout);
			private:
				RenderDevice& render_device;

				// Initial number of descriptor count
				PoolSizes initial_sizes{};

				// Avaliable number of descriptors
				PoolSizes avaliable_sizes{};
				std::vector<DescriptorSet> allocated_descriptors;

				CanAllocateResult last_allocation_result{};

				uint32_t allocated_count{ 0u };
				int32_t countdown_count{ 0 };
			};

			
		}
	}
}