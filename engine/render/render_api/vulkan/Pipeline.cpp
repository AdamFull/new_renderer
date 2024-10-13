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
				create_info.pVertexInputState = &vertex_input_state;
				create_info.pInputAssemblyState = &input_assembly_state;
				create_info.pTessellationState = &tessellation_state;
				create_info.pViewportState = &viewport_state;
				create_info.pRasterizationState = &rasterization_state;
				create_info.pMultisampleState = &multisample_state;
				create_info.pDepthStencilState = &depth_stencil_state;
				create_info.pColorBlendState = &color_blending_state;
				create_info.pDynamicState = &dynamic_state;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaderStage(vk::PipelineShaderStageCreateInfo stage)
			{
				shader_stages.push_back(stage);
				return *this;
			}

			// Vertex input state
			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexInputStateFlags(vk::PipelineVertexInputStateCreateFlags flags)
			{
				vertex_input_state.flags = flags;
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexInputBindingDescription(std::span<vk::VertexInputBindingDescription> binding_description)
			{
				vertex_input_state.pVertexBindingDescriptions = binding_description.data();
				vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_description.size());
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::SetVertexAttributeDescription(std::span<vk::VertexInputAttributeDescription> attribute_description)
			{
				vertex_input_state.pVertexAttributeDescriptions = attribute_description.data();
				vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
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
				viewports.emplace_back(viewport);
				return *this;
			}

			GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddScissorRect(vk::Rect2D rect)
			{
				scissors.emplace_back(rect);
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
				color_blend_attachments.push_back(attachment);
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

			uint32_t GraphicsPipelineBuilder::Hash() const
			{
				uint32_t hash = PipelineBuilder::Hash();

				for (const auto& stage : shader_stages)
				{
					hash = HashShaderStage(stage, hash);
				}

				hash = utl::murmur_hash(static_cast<uint32_t>(vertex_input_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(input_assembly_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(tessellation_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(viewport_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(rasterization_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(multisample_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(depth_stencil_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(color_blending_state.flags), hash);

				hash = utl::murmur_hash(static_cast<uint32_t>(dynamic_state.flags), hash);

				return hash;
			}


			GraphicsPipeline GraphicsPipelineBuilder::Build(RenderDevice& render_device) const
			{
				Finalize();
				return GraphicsPipeline(render_device, *this);
			}

			std::unique_ptr<GraphicsPipeline> GraphicsPipelineBuilder::BuildUnique(RenderDevice& render_device) const
			{
				Finalize();
				return std::make_unique<GraphicsPipeline>(render_device, *this);
			}

			void GraphicsPipelineBuilder::Finalize() const
			{
				create_info.pStages = shader_stages.data();
				create_info.stageCount = static_cast<uint32_t>(shader_stages.size());

				viewport_state.pViewports = viewports.data();
				viewport_state.viewportCount = static_cast<uint32_t>(viewports.size());
				viewport_state.pScissors = scissors.data();
				viewport_state.scissorCount = static_cast<uint32_t>(scissors.size());

				color_blending_state.pAttachments = color_blend_attachments.data();
				color_blending_state.attachmentCount = static_cast<uint32_t>(color_blend_attachments.size());

				dynamic_state.pDynamicStates = dynamic_states.data();
				dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());	
			}

			GraphicsPipeline::GraphicsPipeline(RenderDevice& render_device, const GraphicsPipelineBuilder& builder) :
				Pipeline{ render_device }
			{
				handle = render_device.CreateGraphicsPipeline(builder.create_info, builder.debug_name);
			}
		}
	}
}