#include "gfx_vk_shared_descriptor.h"
#include "gfx_vk_device.h"
#include "gfx_vk_descriptor_pool.h"
#include "gfx_vk_resource_descriptor.h"

#include "gfx_vk_util.h"

#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan
{
	VulkanSharedDescriptor::VulkanSharedDescriptor(Device& device, const SharedDescriptorCreateInfo& create_info) :
		device{ device },
		first_set{ create_info.first_set } {

		DescriptorSetLayoutCreateInfo layout_ci{};
		layout_ci.create_info.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT;

		for (auto& desc : create_info.descriptions) {
			auto& binding = layout_ci.add_binding();
			binding.descriptorType = descriptor_type_to_vk(desc.type);
			binding.binding = desc.binding;
			binding.descriptorCount = desc.count;
			binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;

			auto& flags = layout_ci.add_binding_flags();
			flags = { vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind };
		}

		descriptor_set_layout = std::make_unique<DescriptorSetLayout>(device, layout_ci);

		descriptor_pool = std::make_unique<DescriptorPool>(device, *descriptor_set_layout);
	}

	VulkanSharedDescriptor::~VulkanSharedDescriptor() {
		for (auto& resource_descriptor : allocated_resource_descriptors) {
			delete resource_descriptor;
			resource_descriptor = nullptr;
		}
	}

	VulkanSharedDescriptor::VulkanSharedDescriptor(VulkanSharedDescriptor&& other) :
		device{ other.device },
		first_set{ std::exchange(other.first_set, {}) },
		descriptor_set_layout{ std::exchange(other.descriptor_set_layout , {}) },
		descriptor_pool{ std::exchange(other.descriptor_pool , {}) } {

	}

	void VulkanSharedDescriptor::set_resource_name(std::string_view name) const noexcept {
		assert(false && "NOT IMPLEMENTED");
	}

	void VulkanSharedDescriptor::reset() {
		EDGE_PROFILE_ZONE_SCOPED;

		descriptor_pool->reset();

		for (auto& resource_descriptor : allocated_resource_descriptors) {
			delete resource_descriptor;
			resource_descriptor = nullptr;
		}
		allocated_resource_descriptors.clear();
	}

	auto VulkanSharedDescriptor::get_latest_descriptors() const noexcept -> IResourceDescriptor* {
		return allocated_resource_descriptors.back();
	}

	auto VulkanSharedDescriptor::request_descriptors() -> IResourceDescriptor* {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& new_resource_descriptor = allocated_resource_descriptors.emplace_back(new VulkanResourceDescriptor(device));

		auto& descriptor_set = descriptor_pool->allocate(*descriptor_set_layout);
		new_resource_descriptor->update_descriptor_set(first_set, descriptor_set);

		return new_resource_descriptor;
	}
}