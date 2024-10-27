#include <VkPipeline.h>
#include <VkDevice.h>

#include <VkShader.h>
#include <VkRootSignature.h>
#include <VkPipelineCache.h>

#include <VkUtil.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanPipeline::VulkanPipeline(Device& device) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
			}

			VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) :
				VulkanResource{ std::move(other) },
				device{ device }
			{
			}

			VulkanPipeline::~VulkanPipeline()
			{
				Handle(device).destroyPipeline(handle, allocation_callbacks);
			}



			VulkanComputePipeline::VulkanComputePipeline(Device& device, const ComputePipelineCreateInfo& create_info) :
				VulkanPipeline{ device }
			{
				auto* shader = static_cast<VulkanShader*>(create_info.shader);
				auto* root_signature = static_cast<VulkanRootSignature*>(create_info.root_signature);
				auto* pipeline_cache = static_cast<VulkanPipelineCache*>(create_info.pipeline_cache);

				vk::ComputePipelineCreateInfo pipeline_create_info{};

				auto& shader_modules = shader->GetModules();
				pipeline_create_info.stage = shader_modules.data[0]->GetStageCreateInfo();

				auto& pipeline_layout = root_signature->GetPipelineLayout();
				pipeline_create_info.layout = pipeline_layout.GetHandle();

				vk::PipelineCache cache{ pipeline_cache ? pipeline_cache->GetHandle() : nullptr };
				vk::Result result = Handle(device).createComputePipelines(cache, 1u, &pipeline_create_info, allocation_callbacks, &handle);
			}

			VulkanComputePipeline::VulkanComputePipeline(VulkanComputePipeline&& other) :
				VulkanPipeline{ std::move(other) }
			{

			}

			PipelineType VulkanComputePipeline::GetType() const
			{
				return PipelineType::eCompute;
			}




			VulkanGraphicsPipeline::VulkanGraphicsPipeline(Device& device, const GraphicsPipelineCreateInfo& create_info) :
				VulkanPipeline{ device }
			{
				auto* shader = static_cast<VulkanShader*>(create_info.shader);
				auto* root_signature = static_cast<VulkanRootSignature*>(create_info.root_signature);
				auto* pipeline_cache = static_cast<VulkanPipelineCache*>(create_info.pipeline_cache);

				std::array<vk::PipelineShaderStageCreateInfo, 5ull> shader_stages{};
				vk::GraphicsPipelineCreateInfo pipeline_create_info{};
				pipeline_create_info.pStages = shader_stages.data();

				// Get shader modules
				auto& shader_modules = shader->GetModules();
				for (auto it = shader_modules.Begin(); it != shader_modules.End(); ++it)
				{
					shader_stages[pipeline_create_info.stageCount++] = (*it)->GetStageCreateInfo();
				}

				// Get vertex input
				std::array<vk::VertexInputBindingDescription, 16ull> vertex_input_bindings{};
				std::array<vk::VertexInputAttributeDescription, 16ull> vertex_input_attributes{};
				vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
				pipeline_create_info.pVertexInputState = &vertex_input_state;
				vertex_input_state.pVertexBindingDescriptions = vertex_input_bindings.data();
				vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();

				for (int32_t i = 0; i < create_info.pipeline_state.vertex_layout.binding_count; ++i)
				{
					auto& binding = create_info.pipeline_state.vertex_layout.bindings[i];
					auto& vk_binding = vertex_input_bindings[vertex_input_state.vertexBindingDescriptionCount++];

					vk_binding.binding = binding.binding;
					vk_binding.stride = binding.stride;
					vk_binding.inputRate = vk::VertexInputRate::eVertex;
				}

				for (int32_t i = 0; i < create_info.pipeline_state.vertex_layout.attribute_count; ++i)
				{
					auto& attribute = create_info.pipeline_state.vertex_layout.attributes[i];
					auto& vk_attribute = vertex_input_attributes[vertex_input_state.vertexAttributeDescriptionCount++];

					vk_attribute.location = attribute.location;
					vk_attribute.binding = attribute.binding;
					vk_attribute.format = static_cast<vk::Format>(FormatToVk(attribute.format));
					vk_attribute.offset = attribute.offset;
				}

				// Input assembly state
				vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
				pipeline_create_info.pInputAssemblyState = &input_assembly_state;
				input_assembly_state.topology = PrimitiveTopologyToVk(create_info.pipeline_state.primitive_topology);
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
				rasterization_state.polygonMode = FillModeToVk(create_info.pipeline_state.rasterization_state.fill_mode);
				rasterization_state.cullMode = CullModeToVk(create_info.pipeline_state.rasterization_state.cull_mode);
				rasterization_state.frontFace = FrontFaceToVk(create_info.pipeline_state.rasterization_state.front_face);
				rasterization_state.lineWidth = 1.f;

				// Multisample state
				vk::PipelineMultisampleStateCreateInfo multisample_state{};
				pipeline_create_info.pMultisampleState = &multisample_state;
				multisample_state.rasterizationSamples = SampleCountToVk(create_info.pipeline_state.multisample_state.sample_count);
				multisample_state.alphaToCoverageEnable = create_info.pipeline_state.multisample_state.alpha_to_coverage;
				multisample_state.alphaToOneEnable = create_info.pipeline_state.multisample_state.alpha_to_one;

				// Depth stencil state
				vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};
				pipeline_create_info.pDepthStencilState = &depth_stencil_state;
				depth_stencil_state.depthTestEnable = create_info.pipeline_state.depth_stencil_state.depth_test;
				depth_stencil_state.depthWriteEnable = create_info.pipeline_state.depth_stencil_state.depth_write;
				depth_stencil_state.depthCompareOp = CompareOpToVk(create_info.pipeline_state.depth_stencil_state.depth_compare_op);

				depth_stencil_state.stencilTestEnable = create_info.pipeline_state.depth_stencil_state.stencil_test;
				depth_stencil_state.front.compareOp = CompareOpToVk(create_info.pipeline_state.depth_stencil_state.stencil_front_compare_op);
				depth_stencil_state.front.failOp = StencilOpToVk(create_info.pipeline_state.depth_stencil_state.front_fail);
				depth_stencil_state.front.depthFailOp = StencilOpToVk(create_info.pipeline_state.depth_stencil_state.depth_front_fail);
				depth_stencil_state.front.passOp = StencilOpToVk(create_info.pipeline_state.depth_stencil_state.front_pass);

				depth_stencil_state.back.compareOp = CompareOpToVk(create_info.pipeline_state.depth_stencil_state.stencil_back_compare_op);
				depth_stencil_state.back.failOp = StencilOpToVk(create_info.pipeline_state.depth_stencil_state.back_fail);
				depth_stencil_state.back.depthFailOp = StencilOpToVk(create_info.pipeline_state.depth_stencil_state.depth_back_fail);
				depth_stencil_state.back.passOp = StencilOpToVk(create_info.pipeline_state.depth_stencil_state.back_pass);

				// Color blending state
				std::array<vk::PipelineColorBlendAttachmentState, 8ull> blend_state_attachments{};
				vk::PipelineColorBlendStateCreateInfo color_blending_state{};
				pipeline_create_info.pColorBlendState = &color_blending_state;
				color_blending_state.pAttachments = blend_state_attachments.data();

				for (int32_t i = 0; i < create_info.pipeline_state.color_blending_state.attachment_count; ++i)
				{
					auto& attachment = create_info.pipeline_state.color_blending_state.attachments[i];
					auto& vk_attachment = blend_state_attachments[color_blending_state.attachmentCount++];

					vk_attachment.blendEnable = attachment.enable;
					vk_attachment.srcColorBlendFactor = BlendFactorToVk(attachment.src_color);
					vk_attachment.dstColorBlendFactor = BlendFactorToVk(attachment.dst_color);
					vk_attachment.colorBlendOp = BlendOpToVk(attachment.color_blend_op);
					vk_attachment.srcAlphaBlendFactor = BlendFactorToVk(attachment.src_alpha);
					vk_attachment.dstAlphaBlendFactor = BlendFactorToVk(attachment.dst_alpha);
					vk_attachment.alphaBlendOp = BlendOpToVk(attachment.alpha_blend_op);
					vk_attachment.colorWriteMask = ColorMaskToVk(attachment.color_mask);
				}

				// Dynamic state
				vk::DynamicState dynamic_states[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
				vk::PipelineDynamicStateCreateInfo dynamic_state{};
				pipeline_create_info.pDynamicState = &dynamic_state;
				dynamic_state.pDynamicStates = &dynamic_states[0];
				dynamic_state.dynamicStateCount = 2;

				std::array<vk::Format, 8ull> color_formats{};
				vk::PipelineRenderingCreateInfoKHR dynamic_rendering{};
				pipeline_create_info.pNext = &dynamic_rendering;
				dynamic_rendering.pColorAttachmentFormats = color_formats.data();

				for (int32_t i = 0; i < create_info.pipeline_state.color_format_count; ++i)
				{
					color_formats[dynamic_rendering.colorAttachmentCount++] = static_cast<vk::Format>(FormatToVk(create_info.pipeline_state.color_formats[i]));
				}

				dynamic_rendering.depthAttachmentFormat = static_cast<vk::Format>(FormatToVk(create_info.pipeline_state.depth_format));
				dynamic_rendering.stencilAttachmentFormat = static_cast<vk::Format>(FormatToVk(create_info.pipeline_state.stencil_format));

				auto& pipeline_layout = root_signature->GetPipelineLayout();
				pipeline_create_info.layout = pipeline_layout.GetHandle();

				vk::PipelineCache cache{ pipeline_cache ? pipeline_cache->GetHandle() : nullptr };
				vk::Result result = Handle(device).createGraphicsPipelines(cache, 1u, &pipeline_create_info, allocation_callbacks, &handle);
			}

			VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) :
				VulkanPipeline{ std::move(other) }
			{
			}

			PipelineType VulkanGraphicsPipeline::GetType() const
			{
				return PipelineType::eGraphics;
			}
		}
	}
}