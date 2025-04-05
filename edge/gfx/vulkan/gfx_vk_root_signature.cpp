#include "gfx_vk_root_signature.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_shared_descriptor.h"
#include "gfx_vk_shader.h"

#include "gfx_vk_util.h"

namespace edge::gfx::vulkan
{
	PipelineLayoutCreateInfo::PipelineLayoutCreateInfo() {
		create_info.pSetLayouts = descriptor_set_layouts.data();
		create_info.pPushConstantRanges = push_constant_ranges.data();
	}

	void PipelineLayoutCreateInfo::add_descriptor_set_layout(vk::DescriptorSetLayout layout) {
		descriptor_set_layouts[create_info.setLayoutCount++] = layout;
	}

	vk::PushConstantRange& PipelineLayoutCreateInfo::add_constant_range() {
		return push_constant_ranges[create_info.pushConstantRangeCount++];
	}


	PipelineLayout::PipelineLayout(Device& device, const PipelineLayoutCreateInfo& create_info) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {
		vk::Result result = device.get_handle().createPipelineLayout(&create_info.create_info, allocation_callbacks, &handle_);
	}

	PipelineLayout::~PipelineLayout() {
		device.get_handle().destroyPipelineLayout(handle_, allocation_callbacks);
	}

	PipelineLayout::PipelineLayout(PipelineLayout&& other) :
		Resource{ std::move(other) },
		device{ device } {
	}


	RootSignature::RootSignature(Device& device, const RootSignatureCreateInfo& create_info) :
		device{ device } {
		const auto& reflection = create_info.shader->get_reflection();

		std::array<DescriptorSetLayoutCreateInfo, 8ull> descriptor_set_layout_create_infos{};

		// Create descriptor set layouts
		for (auto it = reflection.shader_resources.begin(); it != reflection.shader_resources.end(); ++it) {
			auto& ci = descriptor_set_layout_create_infos[it->set];

			auto& descriptor_binding = ci.add_binding();
			descriptor_binding.binding = it->binding;
			descriptor_binding.descriptorType = descriptor_type_to_vk(it->type);
			descriptor_binding.descriptorCount = 1u;
			descriptor_binding.stageFlags = shader_stage_flags_to_vk(it->stages);

			auto& flags = ci.add_binding_flags();
		}

		PipelineLayoutCreateInfo layout_ci{};

		// Get constant ranges from reflection
		// TODO: Now i can have only one constant range, but in future i think i can have more then one.
		if (reflection.constants.range_begin != ~0u) {
			auto& range = layout_ci.add_constant_range();
			for (int32_t i = reflection.constants.range_begin; i < reflection.constants.variable_count; ++i) {
				auto& variable = reflection.shader_variables.data[i];
				range.stageFlags = shader_stage_flags_to_vk(reflection.constants.stages);
				range.size += variable.size;
			}
		}

		// Allocate descriptr set layouts
		for (int32_t set = 0; set < 8ull; ++set) {
			// NOTE: it's very dirty solution i think, but ok for now
			if (create_info.shared_descriptor) {
				auto* vk_shared_descriptor = static_cast<SharedDescriptor*>(create_info.shared_descriptor);
				auto shared_set = vk_shared_descriptor->get_first_set();
				if (set == shared_set) {
					auto& shared_layout = vk_shared_descriptor->get_descriptor_set_layout();
					layout_ci.add_descriptor_set_layout(shared_layout.get_handle());
				}
			}

			auto& ci = descriptor_set_layout_create_infos[set];
			if (ci.create_info.bindingCount > 0) {
				descriptor_set_layouts[set] = std::make_unique<DescriptorSetLayout>(device, ci);
				layout_ci.add_descriptor_set_layout(descriptor_set_layouts[set]->get_handle());
			}
		}

		// Add all immutable samplers
		// TODO: immutable samplers creation
		for (int32_t i = 0; i < create_info.immutable_samplers.size(); ++i) {
			assert(false && "NOT IMPLEMENTED");
		}

		pipeline_layout = std::make_unique<PipelineLayout>(device, layout_ci);
	}

	RootSignature::~RootSignature() {
	}

	RootSignature::RootSignature(RootSignature&& other) :
		pipeline_layout{ std::exchange(other.pipeline_layout, {}) },
		device{ other.device } {
	}

	void RootSignature::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(pipeline_layout->get_handle(), name);
	}
}