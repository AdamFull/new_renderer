#pragma once

#include <engine/render/render_api/vulkan/DescriptorSet.h>

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
				std::unique_ptr<DescriptorSet> Allocate(DescriptorSetLayout& layout, const std::string& debug_name = "");
				vk::Result Free(DescriptorSet& descriptor_set);

				bool IsExpired() const;
				CanAllocateResult CanAllocate(DescriptorSetLayout& layout);
			private:
				RenderDevice& render_device;

				// Initial number of descriptor count
				PoolSizes initial_sizes{};

				// Avaliable number of descriptors
				PoolSizes avaliable_sizes{};

				CanAllocateResult last_allocation_result{};

				uint32_t allocated_count{ 0u };
				int32_t countdown_count{ 0 };
			};

			class DescriptorAllocator final
			{
			public:
				DescriptorAllocator(RenderDevice& render_device);
				DescriptorAllocator(const DescriptorAllocator&) = delete;
				DescriptorAllocator(DescriptorAllocator&&);
				~DescriptorAllocator();

				DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
				DescriptorAllocator& operator=(DescriptorAllocator&&) = delete;

				void Reset();
				std::unique_ptr<DescriptorSet> Allocate(DescriptorSetLayout& layout, const std::string& debug_name = "");
			private:
				RenderDevice& render_device;

				// TODO: Do i even need to allocate memory here?
				uint32_t multiplicator{ 2u };
				std::vector<std::unique_ptr<DescriptorPool>> descriptor_pools{};
			};
		}
	}
}