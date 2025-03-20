#include "VkDescriptorSetLayout.h"
#include "VkDevice.h"

namespace edge::gfx::vulkan
{
	DescriptorSetLayoutCreateInfo::DescriptorSetLayoutCreateInfo() {
		ext_create_info.pBindingFlags = binding_flags_ext.data();

		create_info.pBindings = bindings.data();
		create_info.pNext = &ext_create_info;
	}

	vk::DescriptorSetLayoutBinding& DescriptorSetLayoutCreateInfo::add_binding() {
		return bindings[create_info.bindingCount++];
	}

	vk::DescriptorBindingFlagsEXT& DescriptorSetLayoutCreateInfo::add_binding_flags() {
		return binding_flags_ext[ext_create_info.bindingCount++];
	}

	DescriptorSetLayout::DescriptorSetLayout(Device& device, const DescriptorSetLayoutCreateInfo& create_info) :
		VulkanResource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::Result result = device.get_handle().createDescriptorSetLayout(&create_info.create_info, allocation_callbacks, &handle_);

		for (int32_t i = 0; i < create_info.create_info.bindingCount; ++i) {
			auto& binding = create_info.bindings[i];

			// Count number of required descriptors
			pool_sizes[enum_cast(binding.descriptorType)] += binding.descriptorCount;

			layout_bindings[i] = { binding, create_info.binding_flags_ext[i] };
			++binding_count;
		}
	}

	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
		VulkanResource{ std::move(other) },
		device{ other.device },
		layout_bindings{ std::exchange(other.layout_bindings, {}) },
		binding_count{ std::exchange(other.binding_count, {}) },
		pool_sizes{ std::exchange(other.pool_sizes, {}) }
	{
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		device.get_handle()
			.destroyDescriptorSetLayout(handle_, allocation_callbacks);
	}
}