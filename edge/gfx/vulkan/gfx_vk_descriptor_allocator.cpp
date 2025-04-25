#include "gfx_vk_descriptor_allocator.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_device.h"
#include "gfx_vk_root_signature.h"
#include "gfx_vk_descriptor_binding_layout.h"

#include "../../util/Logger.h"
#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan
{
	constexpr const int32_t k_default_pool_multiplier{ 128 };
	constexpr const PoolSizes k_default_pool_sizes {
		4 * k_default_pool_multiplier,		// vk::DescriptorType::eSampler
		4 * k_default_pool_multiplier,		// vk::DescriptorType::eCombinedImageSampler
		4 * k_default_pool_multiplier,		// vk::DescriptorType::eSampledImage
		4 * k_default_pool_multiplier,		// vk::DescriptorType::eStorageImage
		k_default_pool_multiplier,			// vk::DescriptorType::eUniformTexelBuffer
		k_default_pool_multiplier,			// vk::DescriptorType::eStorageTexelBuffer
		2 * k_default_pool_multiplier,		// vk::DescriptorType::eUniformBuffer
		2 * k_default_pool_multiplier,		// vk::DescriptorType::eStorageBuffer
		k_default_pool_multiplier,			// vk::DescriptorType::eUniformBufferDynamic
		k_default_pool_multiplier,			// vk::DescriptorType::eStorageBufferDynamic
		k_default_pool_multiplier			// vk::DescriptorType::eInputAttachment
	};

	DescriptorAllocator::DescriptorAllocator(Device& device, uint32_t grow_factor) :
		device{ device } {
	}

	DescriptorAllocator::DescriptorAllocator(DescriptorAllocator&& other) :
		device{ other.device },
		pool_blocks{ std::exchange(other.pool_blocks, {}) } {
	}

	DescriptorAllocator::~DescriptorAllocator() {
		for (auto& pool : pool_blocks) {
			device.get_handle()
				.destroyDescriptorPool(pool.pool, device.get_allocation_callbacks());
		}
	}

	void DescriptorAllocator::set_resource_name(std::string_view name) const noexcept {
		for (const auto& pool : pool_blocks) {
			auto& debug_utils = device.get_debug_utils();
			debug_utils.set_debug_name(pool.pool, name);
		}
	}

	void DescriptorAllocator::reset() {
		EDGE_PROFILE_ZONE_SCOPED;

		//auto to_delete = std::remove_if(descriptor_pools.begin(), descriptor_pools.end(), 
		//	[](const std::unique_ptr<DescriptorPool>& pool) {
		//		pool->reset();
		//		return pool->is_expired();
		//	});
		//
		//if (to_delete != descriptor_pools.end()) {
		//	descriptor_pools.erase(to_delete);
		//}

		for (auto& pool : pool_blocks) {
			pool.avaliable_sizes = pool.inilial_sizes;
			pool.avaliable_sets = k_default_pool_multiplier;

			device.get_handle().resetDescriptorPool(pool.pool);
		}

		for (auto& layout : allocated_layouts) {
			auto* vk_layout = layout->as<DescriptorBindingLayout*>();
			vk_layout->set_descriptor_set(nullptr, -1);
		}

		allocated_layouts.clear();
	}

	auto DescriptorAllocator::allocate(std::span<IBindingLayout*> layouts) -> bool {
		EDGE_PROFILE_ZONE_SCOPED;

		PoolSizes required_sizes{};
		bool is_bindless_required{ false };
		int32_t descriptor_sets_requested = static_cast<int32_t>(layouts.size());

		std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{ layouts.size() };

		for (int32_t i = 0; i < static_cast<int32_t>(layouts.size()); ++i) {
			auto* vk_binding_layout = layouts[i]->as<DescriptorBindingLayout*>();
			auto* vk_layout = vk_binding_layout->get_descriptor_layout()->as<DescriptorSetLayout*>();
			required_sizes += vk_layout->get_required_descriptor_count();

			descriptor_set_layouts[i] = vk_layout->get_handle();

			if (vk_layout->is_bindless()) {
				is_bindless_required = true;
			}
		}

		auto [pool_obj, pool_idx] = lookup_free_pool(required_sizes, descriptor_sets_requested, is_bindless_required);
		pool_obj.avaliable_sizes -= required_sizes;
		pool_obj.avaliable_sets -= descriptor_sets_requested;

		vk::DescriptorSetAllocateInfo alloc_info{};
		alloc_info.descriptorPool = pool_obj.pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		alloc_info.pSetLayouts = descriptor_set_layouts.data();

		std::vector<vk::DescriptorSet> allocated_descriptor_sets{ layouts.size() };
		vk::Result result = device.get_handle()
			.allocateDescriptorSets(&alloc_info, allocated_descriptor_sets.data());

		if (result != vk::Result::eSuccess) {
			return false;
		}

		for (int32_t i = 0; i < layouts.size(); ++i) {
			auto* vk_binding_layout = layouts[i]->as<DescriptorBindingLayout*>();
			vk_binding_layout->set_descriptor_set(allocated_descriptor_sets[i], static_cast<int32_t>(pool_idx));
		}

		allocated_layouts.insert(allocated_layouts.end(), layouts.begin(), layouts.end());

		return true;
	}

	auto DescriptorAllocator::free(std::span<IBindingLayout*> layouts) -> bool {
		std::vector<std::vector<vk::DescriptorSet>> descriptor_sets{ pool_blocks.size() };

		auto add_deallocate_descriptor = [this, &descriptor_sets](IBindingLayout* layout, vk::DescriptorSet set, int32_t pool_idx) {
			if (!set || pool_idx < 0) {
				EDGE_LOGE("[VK Descriptor Allocator] failed to deallocate descriptor memory for pool \"{}\".", pool_idx);
				return;
			}

			auto* descriptor_layout = layout->get_descriptor_layout();
			auto* vk_descriptor_layout = descriptor_layout->as<DescriptorSetLayout*>();

			auto& pool = pool_blocks[pool_idx];
			pool.avaliable_sets += 1;
			pool.avaliable_sizes += vk_descriptor_layout->get_required_descriptor_count();

			descriptor_sets[pool_idx].push_back(set);
			};

		// Prepare descriptors to utilize
		for (const auto& layout : layouts) {
			auto* vk_binding_layout = layout->as<DescriptorBindingLayout*>();

			// Deallocate last descriptor
			auto [set, pool_idx] = vk_binding_layout->get_descriptor_set();
			add_deallocate_descriptor(layout, set, pool_idx);
			vk_binding_layout->set_descriptor_set(nullptr, -1);
		}

		// Free individual descriptors
		for (int32_t i = 0; i < static_cast<int32_t>(pool_blocks.size()); ++i) {
			auto& pool = pool_blocks[i];

			if (!descriptor_sets[i].empty()) {
				device.get_handle()
					.freeDescriptorSets(pool.pool, static_cast<uint32_t>(descriptor_sets[i].size()), descriptor_sets[i].data());
			}
		}
		
		return true;
	}

	auto DescriptorAllocator::create_new_pool(const PoolSizes& requested_sizes, bool bindless_support) -> DescriptorPoolBlock& {
		auto& new_pool_block = pool_blocks.emplace_back();
		new_pool_block.inilial_sizes = k_default_pool_sizes;
		new_pool_block.avaliable_sizes = k_default_pool_sizes;
		new_pool_block.avaliable_sets = k_default_pool_multiplier;

		if (requested_sizes > new_pool_block.inilial_sizes) {
			new_pool_block.avaliable_sizes = 
				new_pool_block.inilial_sizes = requested_sizes + k_default_pool_sizes;
		}

		std::array<vk::DescriptorPoolSize, k_max_pool_sizes_count> pool_sizes{};
		for (int32_t i = 0; i < k_max_pool_sizes_count; ++i) {
			pool_sizes[i] = { static_cast<vk::DescriptorType>(i), static_cast<uint32_t>(new_pool_block.inilial_sizes[i]) };
		}

		vk::DescriptorPoolCreateInfo create_info{};
		create_info.poolSizeCount = k_max_pool_sizes_count;
		create_info.pPoolSizes = pool_sizes.data();
		create_info.maxSets = k_default_pool_multiplier;
		// For support individual allocations free
		create_info.flags |= vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		if (bindless_support) {
			create_info.flags |= vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT;
			new_pool_block.bindless_support = true;
		}

		vk::Result result = device.get_handle().createDescriptorPool(&create_info, device.get_allocation_callbacks(), &new_pool_block.pool);
		// TODO: check error
		return new_pool_block;
	}

	auto DescriptorAllocator::lookup_free_pool(const PoolSizes& requested_sizes, int32_t requested_set_count, bool bindless_support) -> std::tuple<DescriptorPoolBlock&, size_t> {
		auto found = std::find_if(pool_blocks.begin(), pool_blocks.end(), 
			[&requested_sizes, requested_set_count, bindless_support](const DescriptorPoolBlock& block) {
			return (block.bindless_support == bindless_support) && 
				(block.avaliable_sizes > requested_sizes) && 
				(block.avaliable_sets - requested_set_count >= 0);
			});

		if (found != pool_blocks.end()) {
			return { *found, std::distance(pool_blocks.begin(), found) };
		}

		return { create_new_pool(requested_sizes, bindless_support), pool_blocks .size() - 1};
	}
}