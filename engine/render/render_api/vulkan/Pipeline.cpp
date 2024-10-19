#include <engine/render/render_api/vulkan/Pipeline.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			uint32_t HashShaderStage(const vk::PipelineShaderStageCreateInfo& stage, uint32_t hash)
			{
				hash = utl::murmur_hash(static_cast<uint32_t>(stage.flags), hash);
				hash = utl::murmur_hash(utl::enum_cast(stage.stage), hash);
				hash = utl::murmur_hash(stage.pName, std::strlen(stage.pName), hash);
				hash = HandleHash(stage.module, hash);
				// TODO: Hash specialization info

				return hash;
			}

			uint32_t HashStencilOpState(const vk::StencilOpState& state, uint32_t hash)
			{
				hash = utl::murmur_hash(utl::enum_cast(state.failOp), hash);
				hash = utl::murmur_hash(utl::enum_cast(state.passOp), hash);
				hash = utl::murmur_hash(utl::enum_cast(state.depthFailOp), hash);
				hash = utl::murmur_hash(utl::enum_cast(state.compareOp), hash);
				hash = utl::murmur_hash(state.compareMask, hash);
				hash = utl::murmur_hash(state.writeMask, hash);
				hash = utl::murmur_hash(state.reference, hash);

				return hash;
			}

			Pipeline::Pipeline(RenderDevice& render_device) :
				VulkanResource{ nullptr },
				render_device{ render_device }
			{

			}

			Pipeline::Pipeline(Pipeline&& other) :
				VulkanResource{ std::move(other) },
				render_device{ other.render_device }
			{

			}

			Pipeline::~Pipeline()
			{
				render_device.Destroy(handle);
			}
			
			// Compute pipeline
			ComputePipelineBuilder::ComputePipelineBuilder()
			{

			}

			ComputePipelineBuilder& ComputePipelineBuilder::SetShaderStage(vk::PipelineShaderStageCreateInfo stage)
			{
				create_info.stage = stage;
				return *this;
			}

			uint32_t ComputePipelineBuilder::Hash() const
			{
				uint32_t hash = PipelineBuilder::Hash();

				hash = HashShaderStage(create_info.stage, hash);

				return hash;
			}

			ComputePipeline ComputePipelineBuilder::Build(RenderDevice& render_device) const
			{
				return ComputePipeline(render_device, *this);
			}

			std::unique_ptr<ComputePipeline> ComputePipelineBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<ComputePipeline>(render_device, *this);
			}

			ComputePipeline::ComputePipeline(RenderDevice& render_device, const ComputePipelineBuilder& builder) :
				Pipeline{ render_device }
			{
				handle = render_device.CreateComputePipeline(builder.create_info, builder.debug_name);
			}


			// Graphics pipeline
			GraphicsPipelineBuilder::GraphicsPipelineBuilder()
			{
				create_info.pStages = shader_stages.data();

				vertex_input_state.pVertexBindingDescriptions = binding_descriptions.data();
				vertex_input_state.pVertexAttributeDescriptions = attribute_descriptions.data();
				create_info.pVertexInputState = &vertex_input_state;

				create_info.pInputAssemblyState = &input_assembly_state;
				create_info.pTessellationState = &tessellation_state;

				viewport_state.pViewports = viewports.data();
				viewport_state.pScissors = scissors.data();
				create_info.pViewportState = &viewport_state;

				create_info.pRasterizationState = &rasterization_state;
				create_info.pMultisampleState = &multisample_state;
				create_info.pDepthStencilState = &depth_stencil_state;

				color_blending_state.pAttachments = color_blend_attachments.data();
				create_info.pColorBlendState = &color_blending_state;

				dynamic_state.pDynamicStates = dynamic_states.data();
				create_info.pDynamicState = &dynamic_state;

				dynamic_rendering_create_info.pColorAttachmentFormats = color_attachments.data();
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaderStage(vk::PipelineShaderStageCreateInfo stage)
			{
				shader_stages[create_info.stageCount++] = stage;
				return *this;
			}

			// Vertex input state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexInputStateFlags(vk::PipelineVertexInputStateCreateFlags flags)
			{
				vertex_input_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexInputBindingDescription(const vk::VertexInputBindingDescription& binding_description)
			{
				binding_descriptions[vertex_input_state.vertexBindingDescriptionCount++] = binding_description;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexAttributeDescription(const vk::VertexInputAttributeDescription& attribute_description)
			{
				attribute_descriptions[vertex_input_state.vertexAttributeDescriptionCount++] = attribute_description;
				return *this;
			}


			// Input assembly state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetInputAsseblyStateFlags(vk::PipelineInputAssemblyStateCreateFlags flags)
			{
				input_assembly_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPrimitiveTopology(vk::PrimitiveTopology primitive_topology)
			{
				input_assembly_state.topology = primitive_topology;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPrimitiveRestartEnable(vk::Bool32 enable)
			{
				input_assembly_state.primitiveRestartEnable = enable;
				return *this;
			}


			// Tessellation state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetTessellationStateFlags(vk::PipelineTessellationStateCreateFlags flags)
			{
				tessellation_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPatchControlPointsCount(uint32_t count)
			{
				tessellation_state.patchControlPoints = count;
				return *this;
			}


			// Viewport state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetViewportStateFlags(vk::PipelineViewportStateCreateFlags flags)
			{
				viewport_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddViewport(vk::Viewport viewport)
			{
				viewports[viewport_state.viewportCount++] = viewport;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddScissorRect(vk::Rect2D rect)
			{
				scissors[viewport_state.scissorCount++] = rect;
				return *this;
			}


			// Rasterization state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRasterizationStateFlags(vk::PipelineRasterizationStateCreateFlags flags)
			{
				rasterization_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthClampEnable(vk::Bool32 enable)
			{
				rasterization_state.depthClampEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRasterizerDiscardEnable(vk::Bool32 enable)
			{
				rasterization_state.rasterizerDiscardEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetPolygonMode(vk::PolygonMode mode)
			{
				rasterization_state.polygonMode = mode;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetCullMode(vk::CullModeFlags flags)
			{
				rasterization_state.cullMode = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetFrontFace(vk::FrontFace front_face)
			{
				rasterization_state.frontFace = front_face;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasEnable(vk::Bool32 enable)
			{
				rasterization_state.depthBiasEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasConstantFactor(float factor)
			{
				rasterization_state.depthBiasConstantFactor = factor;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasClamp(float clamp)
			{
				rasterization_state.depthBiasClamp = clamp;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBiasSlopeFactor(float factor)
			{
				rasterization_state.depthBiasSlopeFactor = factor;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetLineWidth(float line_width)
			{
				rasterization_state.lineWidth = line_width;
				return *this;
			}


			// Multisample state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetMultisampleStateFlags(vk::PipelineMultisampleStateCreateFlags flags)
			{
				multisample_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetSampleCount(vk::SampleCountFlagBits sample_count)
			{
				multisample_state.rasterizationSamples = sample_count;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetSampleShadingEnable(vk::Bool32 enable)
			{
				multisample_state.sampleShadingEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetMinSampleShading(float min_sample)
			{
				multisample_state.minSampleShading = min_sample;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetSampleMask(const vk::SampleMask* mask)
			{
				multisample_state.pSampleMask = mask;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetAlphaToCoverageEnable(vk::Bool32 enable)
			{
				multisample_state.alphaToCoverageEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetAlphaToOneEnable(vk::Bool32 enable)
			{
				multisample_state.alphaToOneEnable = enable;
				return *this;
			}


			// Depth stencil state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthStencilStateFlags(vk::PipelineDepthStencilStateCreateFlags flags)
			{
				depth_stencil_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthTestEnable(vk::Bool32 enable)
			{
				depth_stencil_state.depthTestEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthWriteEnable(vk::Bool32 enable)
			{
				depth_stencil_state.depthWriteEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthCompareOp(vk::CompareOp compare_op)
			{
				depth_stencil_state.depthCompareOp = compare_op;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBoundsTestEnable(vk::Bool32 enable)
			{
				depth_stencil_state.depthBoundsTestEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetStencilTestEnable(vk::Bool32 enable)
			{
				depth_stencil_state.stencilTestEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetStencilOpFront(vk::StencilOpState front)
			{
				depth_stencil_state.front = front;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetStencilOpBack(vk::StencilOpState back)
			{
				depth_stencil_state.back = back;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthBounds(float min_depth, float max_depth)
			{
				depth_stencil_state.minDepthBounds = min_depth;
				depth_stencil_state.maxDepthBounds = max_depth;
				return *this;
			}


			// Color blending state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlendingStateFlags(vk::PipelineColorBlendStateCreateFlags flags)
			{
				color_blending_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlendingLogicOpEnable(vk::Bool32 enable)
			{
				color_blending_state.logicOpEnable = enable;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetColorBlendingLogicOp(vk::LogicOp logic_op)
			{
				color_blending_state.logicOp = logic_op;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddColorBlendingAttachment(vk::PipelineColorBlendAttachmentState attachment)
			{
				color_blend_attachments[color_blending_state.attachmentCount++] = attachment;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetBlendConstants(std::array<float, 4> const& constants)
			{
				color_blending_state.blendConstants = constants;
				return *this;
			}


			// Dynamic state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDynamicStateFlags(vk::PipelineDynamicStateCreateFlags flags)
			{
				dynamic_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDynamicState(vk::DynamicState state)
			{
				dynamic_states[dynamic_state.dynamicStateCount++] = state;
				return *this;
			}


			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetRenderPass(vk::RenderPass& render_pass)
			{
				create_info.renderPass = render_pass;
				return *this;
			}

			GraphicsPipelineBuilder GraphicsPipelineBuilder::SetSubpass(uint32_t subpass)
			{
				create_info.subpass = subpass;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddColorAttachmentFormat(vk::Format format)
			{
				color_attachments[dynamic_rendering_create_info.colorAttachmentCount++] = format;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetDepthAttachmentFormat(vk::Format format)
			{
				dynamic_rendering_create_info.depthAttachmentFormat = format;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetStencilAttachmentFormat(vk::Format format)
			{
				dynamic_rendering_create_info.stencilAttachmentFormat = format;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::EnableDynamicRendering(bool enable)
			{
				create_info.pNext = enable ? &dynamic_rendering_create_info : nullptr;
				return *this;
			}

			uint32_t GraphicsPipelineBuilder::Hash() const
			{
				uint32_t hash = PipelineBuilder::Hash();

				for (const auto& stage : shader_stages)
				{
					hash = HashShaderStage(stage, hash);
				}

				// Vertex input state hash
				hash = utl::murmur_hash(static_cast<uint32_t>(vertex_input_state.flags), hash);

				if (vertex_input_state.pVertexBindingDescriptions)
				{
					for (int32_t i = 0; i < vertex_input_state.vertexBindingDescriptionCount; ++i)
					{
						const auto& desc = vertex_input_state.pVertexBindingDescriptions[i];
						hash = utl::murmur_hash(desc.binding, hash);
						hash = utl::murmur_hash(desc.stride, hash);
						hash = utl::murmur_hash(utl::enum_cast(desc.inputRate), hash);
					}
				}

				if (vertex_input_state.pVertexAttributeDescriptions)
				{
					for (int32_t i = 0; i < vertex_input_state.vertexAttributeDescriptionCount; ++i)
					{
						const auto& desc = vertex_input_state.pVertexAttributeDescriptions[i];
						hash = utl::murmur_hash(desc.location, hash);
						hash = utl::murmur_hash(desc.binding, hash);
						hash = utl::murmur_hash(utl::enum_cast(desc.format), hash);
						hash = utl::murmur_hash(desc.offset, hash);
					}
				}

				// Input assembly state hash
				hash = utl::murmur_hash(static_cast<uint32_t>(input_assembly_state.flags), hash);
				hash = utl::murmur_hash(utl::enum_cast(input_assembly_state.topology), hash);
				hash = utl::murmur_hash(input_assembly_state.primitiveRestartEnable, hash);

				// Tessellation state hash
				hash = utl::murmur_hash(static_cast<uint32_t>(tessellation_state.flags), hash);
				hash = utl::murmur_hash(tessellation_state.patchControlPoints, hash);

				// Viewport state hash
				hash = utl::murmur_hash(static_cast<uint32_t>(viewport_state.flags), hash);

				for (const auto& viewport : viewports)
				{
					hash = utl::murmur_hash(viewport.x, hash);
					hash = utl::murmur_hash(viewport.y, hash);
					hash = utl::murmur_hash(viewport.width, hash);
					hash = utl::murmur_hash(viewport.height, hash);
					hash = utl::murmur_hash(viewport.minDepth, hash);
					hash = utl::murmur_hash(viewport.maxDepth, hash);
				}

				for (const auto& scissor : scissors)
				{
					hash = utl::murmur_hash(scissor.offset.x, hash);
					hash = utl::murmur_hash(scissor.offset.y, hash);
					hash = utl::murmur_hash(scissor.extent.width, hash);
					hash = utl::murmur_hash(scissor.extent.height, hash);
				}

				// Rasterization state hash
				hash = utl::murmur_hash(static_cast<uint32_t>(rasterization_state.flags), hash);
				hash = utl::murmur_hash(rasterization_state.depthClampEnable, hash);
				hash = utl::murmur_hash(rasterization_state.rasterizerDiscardEnable, hash);
				hash = utl::murmur_hash(utl::enum_cast(rasterization_state.polygonMode), hash);
				hash = utl::murmur_hash(static_cast<uint32_t>(rasterization_state.cullMode), hash);
				hash = utl::murmur_hash(utl::enum_cast(rasterization_state.frontFace), hash);
				hash = utl::murmur_hash(rasterization_state.depthBiasEnable, hash);
				hash = utl::murmur_hash(rasterization_state.depthBiasConstantFactor, hash);
				hash = utl::murmur_hash(rasterization_state.depthBiasClamp, hash);
				hash = utl::murmur_hash(rasterization_state.depthBiasSlopeFactor, hash);
				hash = utl::murmur_hash(rasterization_state.lineWidth, hash);

				// Multisample state hash
				hash = utl::murmur_hash(static_cast<uint32_t>(multisample_state.flags), hash);
				hash = utl::murmur_hash(utl::enum_cast(multisample_state.rasterizationSamples), hash);
				hash = utl::murmur_hash(multisample_state.sampleShadingEnable, hash);
				hash = utl::murmur_hash(multisample_state.minSampleShading, hash);

				if (multisample_state.pSampleMask)
				{
					hash = utl::murmur_hash(*multisample_state.pSampleMask, hash);
				}
					
				hash = utl::murmur_hash(multisample_state.alphaToCoverageEnable, hash);
				hash = utl::murmur_hash(multisample_state.alphaToOneEnable, hash);

				// Depth state hash
				hash = utl::murmur_hash(static_cast<uint32_t>(depth_stencil_state.flags), hash);
				hash = utl::murmur_hash(depth_stencil_state.depthTestEnable, hash);
				hash = utl::murmur_hash(depth_stencil_state.depthWriteEnable, hash);
				hash = utl::murmur_hash(utl::enum_cast(depth_stencil_state.depthCompareOp), hash);
				hash = utl::murmur_hash(depth_stencil_state.depthBoundsTestEnable, hash);
				hash = utl::murmur_hash(depth_stencil_state.stencilTestEnable, hash);
				hash = HashStencilOpState(depth_stencil_state.front, hash);
				hash = HashStencilOpState(depth_stencil_state.back, hash);
				hash = utl::murmur_hash(depth_stencil_state.minDepthBounds, hash);
				hash = utl::murmur_hash(depth_stencil_state.maxDepthBounds, hash);

				// Color blending hash
				hash = utl::murmur_hash(static_cast<uint32_t>(color_blending_state.flags), hash);
				hash = utl::murmur_hash(color_blending_state.logicOpEnable, hash);
				hash = utl::murmur_hash(utl::enum_cast(color_blending_state.logicOp), hash);

				for (const auto& attachment : color_blend_attachments)
				{
					hash = utl::murmur_hash(attachment.blendEnable, hash);
					hash = utl::murmur_hash(utl::enum_cast(attachment.srcColorBlendFactor), hash);
					hash = utl::murmur_hash(utl::enum_cast(attachment.dstColorBlendFactor), hash);
					hash = utl::murmur_hash(utl::enum_cast(attachment.colorBlendOp), hash);
					hash = utl::murmur_hash(utl::enum_cast(attachment.srcAlphaBlendFactor), hash);
					hash = utl::murmur_hash(utl::enum_cast(attachment.dstAlphaBlendFactor), hash);
					hash = utl::murmur_hash(utl::enum_cast(attachment.alphaBlendOp), hash);
					hash = utl::murmur_hash(static_cast<uint32_t>(attachment.colorWriteMask), hash);
				}

				for(const auto& constant : color_blending_state.blendConstants)
					hash = utl::murmur_hash(constant, hash);

				// Dynamic states hash
				hash = utl::murmur_hash(static_cast<uint32_t>(dynamic_state.flags), hash);
				for(const auto& state : dynamic_states)
					hash = utl::murmur_hash(utl::enum_cast(state), hash);

				return hash;
			}


			GraphicsPipeline GraphicsPipelineBuilder::Build(RenderDevice& render_device) const
			{
				return GraphicsPipeline(render_device, *this);
			}

			std::unique_ptr<GraphicsPipeline> GraphicsPipelineBuilder::BuildUnique(RenderDevice& render_device) const
			{
				return std::make_unique<GraphicsPipeline>(render_device, *this);
			}

			GraphicsPipeline::GraphicsPipeline(RenderDevice& render_device, const GraphicsPipelineBuilder& builder) :
				Pipeline{ render_device }
			{
				handle = render_device.CreateGraphicsPipeline(builder.create_info, builder.debug_name);
			}
		}
	}
}