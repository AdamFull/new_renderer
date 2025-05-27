#include "gfx_vk_pipeline.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "gfx_vk_shader.h"
#include "gfx_vk_root_signature.h"
#include "gfx_vk_pipeline_cache.h"

#include "gfx_vk_util.h"

namespace edge::gfx::vulkan
{
	Pipeline::Pipeline(Device& device) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {
	}

	Pipeline::Pipeline(Pipeline&& other) :
		Resource{ std::move(other) },
		device{ device } {
	}

	Pipeline::~Pipeline() {
		device.get_handle().destroyPipeline(handle_, allocation_callbacks);
	}

	void Pipeline::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}



	ComputePipeline::ComputePipeline(Device& device, const ComputePipelineCreateInfo& create_info) :
		Pipeline{ device } {
		auto* shader = utl::cast<Shader*>(create_info.shader);
		auto* root_signature = utl::cast<RootSignature*>(create_info.root_signature);
		auto* pipeline_cache = utl::cast<PipelineCache*>(create_info.pipeline_cache);

		vk::ComputePipelineCreateInfo pipeline_create_info{};

		auto shader_modules = shader->get_modules();
		pipeline_create_info.stage = shader_modules[0]->get_stage_create_info();

		auto& pipeline_layout = root_signature->get_pipeline_layout();
		pipeline_create_info.layout = pipeline_layout.get_handle();

		vk::PipelineCache cache{ pipeline_cache ? pipeline_cache->get_handle() : nullptr };
		vk::Result result = device.get_handle().createComputePipelines(cache, 1u, &pipeline_create_info, allocation_callbacks, &handle_);
	}

	ComputePipeline::ComputePipeline(ComputePipeline&& other) :
		Pipeline{ std::move(other) } {
	}




	GraphicsPipeline::GraphicsPipeline(Device& device, const GraphicsPipelineCreateInfo& create_info) :
		Pipeline{ device } {
		auto* shader = utl::cast<Shader*>(create_info.shader);
		auto* root_signature = utl::cast<RootSignature*>(create_info.root_signature);
		auto* pipeline_cache = utl::cast<PipelineCache*>(create_info.pipeline_cache);

		std::array<vk::PipelineShaderStageCreateInfo, 5ull> shader_stages{};
		vk::GraphicsPipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.pStages = shader_stages.data();

		// Get shader modules
		auto shader_modules = shader->get_modules();
		for (const auto& sm : shader_modules) {
			shader_stages[pipeline_create_info.stageCount++] = sm->get_stage_create_info();
		}

		// Get vertex input
		std::array<vk::VertexInputBindingDescription, 16ull> vertex_input_bindings{};
		std::array<vk::VertexInputAttributeDescription, 16ull> vertex_input_attributes{};
		vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
		pipeline_create_info.pVertexInputState = &vertex_input_state;
		vertex_input_state.pVertexBindingDescriptions = vertex_input_bindings.data();
		vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();

		for (const auto& binding : create_info.pipeline_state.vertex_layout.bindings) {
			auto& vk_binding = vertex_input_bindings[vertex_input_state.vertexBindingDescriptionCount++];

			vk_binding.binding = binding.binding;
			vk_binding.stride = binding.stride;
			vk_binding.inputRate = vk::VertexInputRate::eVertex;
		}

		for (const auto& attribute : create_info.pipeline_state.vertex_layout.attributes) {
			auto& vk_attribute = vertex_input_attributes[vertex_input_state.vertexAttributeDescriptionCount++];

			vk_attribute.location = attribute.location;
			vk_attribute.binding = attribute.binding;
			vk_attribute.format = static_cast<vk::Format>(format_to_vk(attribute.format));
			vk_attribute.offset = attribute.offset;
		}

		// Input assembly state
		vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
		pipeline_create_info.pInputAssemblyState = &input_assembly_state;
		input_assembly_state.topology = primitive_topology_to_vk(create_info.pipeline_state.primitive_topology);
		input_assembly_state.primitiveRestartEnable = VK_FALSE;

		// Tessellation state
		vk::PipelineTessellationStateCreateInfo tessellation_state{};
		pipeline_create_info.pTessellationState = &tessellation_state;

		// Skipping viewport state because it's dynamic state
		vk::PipelineViewportStateCreateInfo viewport_state{};
		pipeline_create_info.pViewportState = &viewport_state;
		viewport_state.viewportCount = 1u;
		viewport_state.scissorCount = 1u;

		// Rasterization state
		vk::PipelineRasterizationStateCreateInfo rasterization_state{};
		pipeline_create_info.pRasterizationState = &rasterization_state;
		rasterization_state.depthClampEnable = create_info.pipeline_state.rasterization_state.depth_clamp;
		rasterization_state.rasterizerDiscardEnable = create_info.pipeline_state.rasterization_state.discard_enable;
		rasterization_state.polygonMode = fill_mode_to_vk(create_info.pipeline_state.rasterization_state.fill_mode);
		rasterization_state.cullMode = cull_mode_to_vk(create_info.pipeline_state.rasterization_state.cull_mode);
		rasterization_state.frontFace = front_face_to_vk(create_info.pipeline_state.rasterization_state.front_face);
		rasterization_state.lineWidth = 1.f;

		// Multisample state
		vk::PipelineMultisampleStateCreateInfo multisample_state{};
		pipeline_create_info.pMultisampleState = &multisample_state;
		multisample_state.rasterizationSamples = sample_count_to_vk(create_info.pipeline_state.multisample_state.sample_count);
		multisample_state.alphaToCoverageEnable = create_info.pipeline_state.multisample_state.alpha_to_coverage;
		multisample_state.alphaToOneEnable = create_info.pipeline_state.multisample_state.alpha_to_one;

		// Depth stencil state
		vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};
		pipeline_create_info.pDepthStencilState = &depth_stencil_state;
		depth_stencil_state.depthTestEnable = create_info.pipeline_state.depth_stencil_state.depth_test;
		depth_stencil_state.depthWriteEnable = create_info.pipeline_state.depth_stencil_state.depth_write;
		depth_stencil_state.depthCompareOp = compare_op_to_vk(create_info.pipeline_state.depth_stencil_state.depth_compare_op);

		depth_stencil_state.stencilTestEnable = create_info.pipeline_state.depth_stencil_state.stencil_test;
		depth_stencil_state.front.compareOp = compare_op_to_vk(create_info.pipeline_state.depth_stencil_state.stencil_front_compare_op);
		depth_stencil_state.front.failOp = stencil_op_to_vk(create_info.pipeline_state.depth_stencil_state.front_fail);
		depth_stencil_state.front.depthFailOp = stencil_op_to_vk(create_info.pipeline_state.depth_stencil_state.depth_front_fail);
		depth_stencil_state.front.passOp = stencil_op_to_vk(create_info.pipeline_state.depth_stencil_state.front_pass);

		depth_stencil_state.back.compareOp = compare_op_to_vk(create_info.pipeline_state.depth_stencil_state.stencil_back_compare_op);
		depth_stencil_state.back.failOp = stencil_op_to_vk(create_info.pipeline_state.depth_stencil_state.back_fail);
		depth_stencil_state.back.depthFailOp = stencil_op_to_vk(create_info.pipeline_state.depth_stencil_state.depth_back_fail);
		depth_stencil_state.back.passOp = stencil_op_to_vk(create_info.pipeline_state.depth_stencil_state.back_pass);

		// Color blending state
		std::array<vk::PipelineColorBlendAttachmentState, 8ull> blend_state_attachments{};
		vk::PipelineColorBlendStateCreateInfo color_blending_state{};
		pipeline_create_info.pColorBlendState = &color_blending_state;
		color_blending_state.pAttachments = blend_state_attachments.data();

		for (const auto& attachment : create_info.pipeline_state.color_blending_state.attachments) {
			auto& vk_attachment = blend_state_attachments[color_blending_state.attachmentCount++];

			vk_attachment.blendEnable = attachment.enable;
			vk_attachment.srcColorBlendFactor = blend_factor_to_vk(attachment.src_color);
			vk_attachment.dstColorBlendFactor = blend_factor_to_vk(attachment.dst_color);
			vk_attachment.colorBlendOp = blend_op_to_vk(attachment.color_blend_op);
			vk_attachment.srcAlphaBlendFactor = blend_factor_to_vk(attachment.src_alpha);
			vk_attachment.dstAlphaBlendFactor = blend_factor_to_vk(attachment.dst_alpha);
			vk_attachment.alphaBlendOp = blend_op_to_vk(attachment.alpha_blend_op);
			vk_attachment.colorWriteMask = color_mask_to_vk(attachment.color_mask);
		}

		// Dynamic state
		vk::DynamicState dynamic_states[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamic_state{};
		pipeline_create_info.pDynamicState = &dynamic_state;
		dynamic_state.pDynamicStates = &dynamic_states[0];
		dynamic_state.dynamicStateCount = 2;

		std::array<vk::Format, 8ull> color_formats{};
		vk::PipelineRenderingCreateInfoKHR dynamic_rendering{};
		pipeline_create_info.pNext = &dynamic_rendering;
		dynamic_rendering.pColorAttachmentFormats = color_formats.data();

		for (auto format : create_info.color_formats) {
			color_formats[dynamic_rendering.colorAttachmentCount++] = static_cast<vk::Format>(format_to_vk(format));
		}

		dynamic_rendering.depthAttachmentFormat = static_cast<vk::Format>(format_to_vk(create_info.depth_format));
		dynamic_rendering.stencilAttachmentFormat = static_cast<vk::Format>(format_to_vk(create_info.stencil_format));

		auto& pipeline_layout = root_signature->get_pipeline_layout();
		pipeline_create_info.layout = pipeline_layout.get_handle();

		vk::PipelineCache cache{ pipeline_cache ? pipeline_cache->get_handle() : nullptr };
		vk::Result result = device.get_handle().createGraphicsPipelines(cache, 1u, &pipeline_create_info, allocation_callbacks, &handle_);
	}

	GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) :
		Pipeline{ std::move(other) } {
	}
}