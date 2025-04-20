#include "gfx_vk_root_signature.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_shader.h"

#include "gfx_vk_util.h"

#include "../../util/Logger.h"

namespace edge::gfx::vulkan
{
	PipelineLayoutCreateInfo::PipelineLayoutCreateInfo() {
		create_info.pSetLayouts = descriptor_set_layouts.data();
		create_info.pPushConstantRanges = push_constant_ranges.data();
	}

	void PipelineLayoutCreateInfo::add_descriptor_set_layout(uint32_t set, vk::DescriptorSetLayout layout) {
		descriptor_set_layouts[set] = layout;
		create_info.setLayoutCount = set + 1;
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

		PipelineLayoutCreateInfo layout_ci{};

		// Get constant ranges from reflection
		// TODO: Now i can have only one constant range, but in future i think i can have more then one.
		if (reflection.constants.range_begin != ~0u) {
			auto& range = layout_ci.add_constant_range();
			for (int32_t i = reflection.constants.range_begin; i < reflection.constants.variable_count; ++i) {
				auto& variable = reflection.shader_variables.data[i];
				range.stageFlags = shader_stage_flags_to_vk(reflection.constants.stages);
				range.size += variable.size;

				self_hash_ = murmur_hash(static_cast<int32_t>(reflection.constants.stages), self_hash_);
				self_hash_ = murmur_hash(variable.size, self_hash_);
			}
		}

		for (int32_t set = 0; set < create_info.binding_layouts.size(); ++set) {
			auto* vk_descriptor_set_layout = utl::cast<DescriptorSetLayout*>(create_info.binding_layouts[set]);
			self_hash_ += vk_descriptor_set_layout->hash();
			if (!vk_descriptor_set_layout) {
				EDGE_LOGE("[Root Signature] missing set slot {}. Cannot create root signature with missing descriptor layout.", set);
				continue;
			}
			layout_ci.add_descriptor_set_layout(set, vk_descriptor_set_layout->get_handle());
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

	auto RootSignature::hash() const noexcept -> uint32_t {
		return self_hash_;
	}
}