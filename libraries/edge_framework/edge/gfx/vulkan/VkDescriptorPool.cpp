#include "VkDescriptorPool.h"
#include "VkDevice.h"

#include "../../util/Profiler.h"

namespace edge::gfx::vulkan
{
	constexpr size_t k_max_sets{ 50ull };
	constexpr size_t k_max_frames_unused{ 300ull };

	PoolSizes k_default_pool_sizes {
		1u, // vk::DescriptorType::eSampler
		4u, // vk::DescriptorType::eCombinedImageSampler
		4u, // vk::DescriptorType::eSampledImage
		1u, // vk::DescriptorType::eStorageImage
		1u, // vk::DescriptorType::eUniformTexelBuffer
		1u, // vk::DescriptorType::eStorageTexelBuffer
		2u, // vk::DescriptorType::eUniformBuffer
		2u, // vk::DescriptorType::eStorageBuffer
		1u, // vk::DescriptorType::eUniformBufferDynamic
		1u, // vk::DescriptorType::eStorageBufferDynamic
		1u  // vk::DescriptorType::eInputAttachment
	};

	PoolSizes merge_pool_sizes(const PoolSizes& lhs, const PoolSizes& rhs, uint32_t mult = 1u) {
		PoolSizes new_sizes{ lhs };

		for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
			new_sizes[i] = (new_sizes[i] + rhs[i]) * mult;

		return new_sizes;
	}

	bool have_enought_sizes(const PoolSizes& lhs, const PoolSizes& rhs) {
		for (int32_t i = 0; i < k_max_pool_sizes_count; ++i) {
			if (lhs[i] - rhs[i] < 0)
				return false;
		}

		return true;
	}

	DescriptorPool::DescriptorPool(Device& device, DescriptorSetLayout& layout, DescriptorPool* previous_pool) :
		VulkanResource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {
		auto& requested_sizes = layout.get_required_descriptor_count();

		if (previous_pool) {
			switch (previous_pool->last_allocation_result) {
			case CanAllocateResult::MaxSetsLimit:
				std::copy(previous_pool->initial_sizes.begin(), previous_pool->initial_sizes.end(), initial_sizes.begin());
				break;
			case CanAllocateResult::MaxDescriptorLimit:
				initial_sizes = merge_pool_sizes(previous_pool->initial_sizes, requested_sizes, 2);
				break;
			case CanAllocateResult::InvalidFlags: break;
			default: break;
			}
		}
		else {
			initial_sizes = merge_pool_sizes(k_default_pool_sizes, requested_sizes, 2);
		}

		// Reset avaliable sizes
		std::copy(initial_sizes.begin(), initial_sizes.end(), avaliable_sizes.begin());

		// Prepare pool sizes
		std::array<vk::DescriptorPoolSize, k_max_pool_sizes_count> pool_sizes{};
		for (int32_t i = 0; i < k_max_pool_sizes_count; ++i) {
			pool_sizes[i] = { static_cast<vk::DescriptorType>(i), static_cast<uint32_t>(initial_sizes[i]) };
		}

		vk::DescriptorPoolCreateInfo create_info{};
		create_info.poolSizeCount = k_max_pool_sizes_count;
		create_info.pPoolSizes = pool_sizes.data();
		create_info.maxSets = k_max_sets;

		for (int32_t i = 0; i < layout.get_binding_count(); ++i) {
			auto& flags = layout.get_layout_binding_flags(i);
			if (flags & vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind) {
				create_info.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT;
				break;
			}
		}

		vk::Result result = device.get_handle().createDescriptorPool(&create_info, allocation_callbacks, &handle_);
		countdown_count = k_max_frames_unused;
	}

	DescriptorPool::DescriptorPool(DescriptorPool&& other) :
		VulkanResource{ std::move(other) },
		device{ other.device },
		initial_sizes{ std::exchange(other.initial_sizes, {}) },
		avaliable_sizes{ std::exchange(other.avaliable_sizes, {}) },
		allocated_count{ std::exchange(other.allocated_count, {}) },
		countdown_count{ std::exchange(other.countdown_count, {}) },
		allocated_descriptors{ std::exchange(other.allocated_descriptors, {}) } {

		allocated_descriptors.resize(k_max_sets);
	}

	DescriptorPool::~DescriptorPool() {
		device.get_handle()
			.destroyDescriptorPool(handle_, allocation_callbacks);
	}

	void DescriptorPool::reset() {
		EDGE_PROFILE_ZONE_SCOPED;

		device.get_handle().resetDescriptorPool(handle_);
		allocated_descriptors.clear();

		last_allocation_result = CanAllocateResult::Success;
		allocated_count = 0u;
		--countdown_count;

		// Reset avaliable descriptors
		std::copy(initial_sizes.begin(), initial_sizes.end(), avaliable_sizes.begin());
	}

	auto DescriptorPool::allocate(DescriptorSetLayout& layout, std::string_view debug_name) -> DescriptorSet& {
		EDGE_PROFILE_ZONE_SCOPED;

		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info.descriptorPool = handle_;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &layout.get_handle();

		vk::DescriptorSet descriptor_set{ VK_NULL_HANDLE };
		vk::Result result = device.get_handle()
			.allocateDescriptorSets(&alloc_info, &descriptor_set);

		assert(descriptor_set);

		auto& required_descriptor_count = layout.get_required_descriptor_count();
		for (int32_t i = 0; i < k_max_pool_sizes_count; ++i) {
			avaliable_sizes[i] -= required_descriptor_count[i];
		}
		++allocated_count;
		countdown_count = k_max_frames_unused;

		return allocated_descriptors.emplace_back(descriptor_set, required_descriptor_count);
	}

	auto DescriptorPool::free(DescriptorSet& descriptor_set) -> vk::Result {
		EDGE_PROFILE_ZONE_SCOPED;

		if (descriptor_set.handle_) {
			device.get_handle().freeDescriptorSets(handle_, 1u, &descriptor_set.handle_);
			descriptor_set.handle_ = nullptr;

			--allocated_count;
			countdown_count = k_max_frames_unused;

			for (int32_t i = 0; i < k_max_pool_sizes_count; ++i) {
				avaliable_sizes[i] += descriptor_set.used_sizes[i];
			}

			return vk::Result::eSuccess;
		}

		return vk::Result::eIncomplete;
	}

	auto DescriptorPool::can_allocate(DescriptorSetLayout& layout) noexcept -> CanAllocateResult {
		auto& required_descriptor_count = layout.get_required_descriptor_count();

		bool max_set_limit_reached = allocated_count >= k_max_sets;
		bool max_descriptor_limit_reached = !have_enought_sizes(avaliable_sizes, required_descriptor_count);

		last_allocation_result = CanAllocateResult::Success;
		if (!max_descriptor_limit_reached && max_set_limit_reached)
			last_allocation_result = CanAllocateResult::MaxSetsLimit;
		else if (max_descriptor_limit_reached)
			last_allocation_result = CanAllocateResult::MaxSetsLimit;

		// TODO: implement flags and other

		return last_allocation_result;
	}
}