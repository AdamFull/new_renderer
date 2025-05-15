#include "gfx_vk_descriptor_set_layout.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_device.h"

#include "gfx_vk_util.h"

#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan
{
	DescriptorSetLayout::DescriptorSetLayout(Device& device, const DescriptorLayoutCreateInfo& layout_ci) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {
		vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT ext_create_info{};
		vk::DescriptorSetLayoutCreateInfo create_info{};
		create_info.pNext = &ext_create_info;

		std::vector<vk::DescriptorBindingFlagsEXT> binding_flags{};
		std::vector<vk::DescriptorSetLayoutBinding> bindings{};

		for (const auto& layout_binding : layout_ci.bindings) {
			hash_combine(murmur_hash(layout_binding.binding), resource_hash);
			hash_combine(murmur_hash(layout_binding.count), resource_hash);
			hash_combine(murmur_hash(layout_binding.type), resource_hash);
			hash_combine(layout_binding.name_hash, resource_hash);

			auto& new_binding = bindings.emplace_back();
			new_binding.descriptorType = descriptor_type_to_vk(layout_binding.type);
			new_binding.binding = layout_binding.binding;
			new_binding.descriptorCount = layout_binding.count;
			new_binding.stageFlags = shader_stage_flags_to_vk(layout_binding.stages);

			auto& new_flags = binding_flags.emplace_back();
			new_flags |= vk::DescriptorBindingFlagBits::ePartiallyBound;
			if (layout_ci.support_bindless) {
				new_flags |= vk::DescriptorBindingFlagBits::eUpdateAfterBind;
			}

			pool_sizes.sizes[utl::cast(new_binding.descriptorType)] += new_binding.descriptorCount;
		}

		binding_count = static_cast<uint32_t>(layout_ci.bindings.size());

		ext_create_info.pBindingFlags = binding_flags.data();
		ext_create_info.bindingCount = static_cast<uint32_t>(binding_flags.size());

		if (layout_ci.support_bindless) {
			have_bindless_descriptor = true;
			create_info.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
		}

		create_info.pBindings = bindings.data();
		
		create_info.bindingCount = static_cast<uint32_t>(bindings.size());

		// TODO: Add error check
		vk::Result result = device.get_handle().createDescriptorSetLayout(&create_info, allocation_callbacks, &handle_);
	}

	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
		Resource{ std::move(other) },
		device{ other.device },
		binding_count{ std::exchange(other.binding_count, {}) },
		pool_sizes{ std::exchange(other.pool_sizes, {}) } {
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		device.get_handle()
			.destroyDescriptorSetLayout(handle_, allocation_callbacks);
	}

	void DescriptorSetLayout::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	auto DescriptorSetLayout::is_bindless() const noexcept -> bool {
		return have_bindless_descriptor;
	}
}