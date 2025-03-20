#pragma once

#include "VkDescriptorSet.h"

namespace edge::gfx::vulkan
{
	enum class CanAllocateResult {
		Success,
		MaxSetsLimit,
		MaxDescriptorLimit,
		InvalidFlags,
		Other
	};

	class DescriptorPool final : public VulkanResource<vk::DescriptorPool> {
	public:
		DescriptorPool(Device& device, DescriptorSetLayout& layout, DescriptorPool* previous_pool = nullptr);
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&);
		~DescriptorPool();

		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) = delete;

		void reset();
		auto allocate(DescriptorSetLayout& layout, std::string_view debug_name = "") -> DescriptorSet&;
		auto free(DescriptorSet& descriptor_set) -> vk::Result;

		auto is_expired() const noexcept -> bool {
			return countdown_count < 0;
		}

		auto can_allocate(DescriptorSetLayout& layout) noexcept -> CanAllocateResult;
	private:
		Device& device;

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