#include "gfx_vk_descriptor_allocator.h"
#include "gfx_vk_device.h"
#include "gfx_vk_descriptor_pool.h"
#include "gfx_vk_root_signature.h"
#include "gfx_vk_resource_descriptor.h"

#include "../../util/Profiler.h"

namespace edge::gfx::vulkan
{
	VulkanDescriptorAllocator::VulkanDescriptorAllocator(Device& device, uint32_t grow_factor) :
		device{ device },
		multiplicator{ grow_factor } {
	}

	VulkanDescriptorAllocator::VulkanDescriptorAllocator(VulkanDescriptorAllocator&& other) :
		device{ other.device },
		descriptor_pools{ std::exchange(other.descriptor_pools, {}) } {
	}

	VulkanDescriptorAllocator::~VulkanDescriptorAllocator() {
		descriptor_pools.clear();

		for (auto& descriptor : allocated_descriptors) {
			delete descriptor;
			descriptor = nullptr;
		}
	}

	void VulkanDescriptorAllocator::set_resource_name(std::string_view name) const noexcept {
		assert(false && "NOT IMPLEMENTED");
	}

	void VulkanDescriptorAllocator::reset() {
		EDGE_PROFILE_ZONE_SCOPED;

		auto to_delete = std::remove_if(descriptor_pools.begin(), descriptor_pools.end(), 
			[](const std::unique_ptr<DescriptorPool>& pool) {
				pool->reset();
				return pool->is_expired();
			});

		if (to_delete != descriptor_pools.end()) {
			descriptor_pools.erase(to_delete);
		}

		for (auto& descriptor : allocated_descriptors) {
			delete descriptor;
			descriptor = nullptr;
		}

		allocated_descriptors.clear();
	}

	auto VulkanDescriptorAllocator::allocate_descriptors(IRootSignature* root_signature) -> IResourceDescriptor* {
		EDGE_PROFILE_ZONE_SCOPED;

		auto* vk_root_signature = static_cast<VulkanRootSignature*>(root_signature);
		auto& set_layouts = vk_root_signature->get_descriptor_set_layouts();

		auto& new_resource_descriptor = allocated_descriptors.emplace_back(new VulkanResourceDescriptor(device));
		for (int32_t i = 0; i < k_max_descriptor_count; ++i) {
			// TODO: check is uses external set layout (ignore externals)

			auto& layout = set_layouts[i];
			if (layout) {
				auto& new_descriptor_set = allocate(*layout);
				new_resource_descriptor->update_descriptor_set(i, new_descriptor_set);
			}
		}

		return new_resource_descriptor;
	}

	auto VulkanDescriptorAllocator::allocate(DescriptorSetLayout& layout) -> DescriptorSet& {
		EDGE_PROFILE_ZONE_SCOPED;

		// We didn't have any pool for now, needs to allocate new one
		if (descriptor_pools.empty()) {
			auto& new_pool = descriptor_pools.emplace_back(std::make_unique<DescriptorPool>(device, layout));
			return new_pool->allocate(layout);
		}

		const auto& required_descriptor_count = layout.get_required_descriptor_count();
		auto found = std::find_if(descriptor_pools.begin(), descriptor_pools.end(), 
			[&layout](const std::unique_ptr<DescriptorPool>& pool) {
				return pool->can_allocate(layout) == CanAllocateResult::Success;
			});

		if (found != descriptor_pools.end()) {
			return (*found)->allocate(layout);
		}

		auto& new_pool = descriptor_pools.emplace_back(std::make_unique<DescriptorPool>(device, layout, !descriptor_pools.empty() ? descriptor_pools.back().get() : nullptr));
		return new_pool->allocate(layout);
	}
}