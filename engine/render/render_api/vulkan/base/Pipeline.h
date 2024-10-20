#pragma once

#include <VkResource.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;

			constexpr uint32_t k_max_render_target_count{ 8u };
			constexpr uint32_t k_max_shader_stage_count{ 8u };

			template<typename _CreateInfoType, typename _BuilderType>
			class PipelineBuilder
			{
			public:
				std::string debug_name{};
				_CreateInfoType create_info{};

				_BuilderType& SetDebugName(const std::string& name)
				{
					debug_name = name;
					return *static_cast<_BuilderType*>(this);
				}

				_BuilderType& SetFlags(vk::PipelineCreateFlags flags)
				{
					create_info.flags = flags;
					return *static_cast<_BuilderType*>(this);
				}

				_BuilderType& SetPipelineLayout(vk::PipelineLayout& layout)
				{
					create_info.layout = layout;
					return *static_cast<_BuilderType*>(this);
				}

				_BuilderType& SetBasePipeline(vk::Pipeline& pipeline)
				{
					create_info.basePipelineHandle = pipeline;
					return *static_cast<_BuilderType*>(this);
				}

				_BuilderType& SetBasePipelineIndex(int32_t index)
				{
					create_info.basePipelineIndex = index;
					return *static_cast<_BuilderType*>(this);
				}

				uint32_t Hash() const
				{
					uint32_t hash{ 0u };

					hash = utl::murmur_hash(debug_name, hash);
					hash = utl::murmur_hash(static_cast<uint32_t>(create_info.flags), hash);
					hash = HandleHash(create_info.layout, hash);
					hash = HandleHash(create_info.basePipelineHandle, hash);
					hash = utl::murmur_hash(create_info.basePipelineIndex, hash);

					return hash;
				}
			};

			class Pipeline : public VulkanResource<vk::Pipeline>
			{
			public:
				Pipeline(RenderDevice& render_device);

				Pipeline(const Pipeline&) = delete;

				Pipeline(Pipeline&& other);

				virtual ~Pipeline();

				Pipeline& operator=(const Pipeline&) = delete;

				Pipeline& operator=(Pipeline&&) = delete;
			protected:
				RenderDevice& render_device;
			};

			class ComputePipeline;

			class ComputePipelineBuilder : public PipelineBuilder<vk::ComputePipelineCreateInfo, ComputePipelineBuilder>
			{
			public:
				ComputePipelineBuilder();

				ComputePipelineBuilder& SetShaderStage(vk::PipelineShaderStageCreateInfo stage);

				uint32_t Hash() const;

				ComputePipeline Build(RenderDevice& render_device) const;
				std::unique_ptr<ComputePipeline> BuildUnique(RenderDevice& render_device) const;
			};

			class ComputePipeline : public Pipeline
			{
			public:
				ComputePipeline(ComputePipeline&&) = default;

				virtual ~ComputePipeline() = default;

				ComputePipeline(RenderDevice& render_device, const ComputePipelineBuilder& builder);
			};

			class GraphicsPipeline;

			class GraphicsPipelineBuilder : public PipelineBuilder<vk::GraphicsPipelineCreateInfo, GraphicsPipelineBuilder>
			{
			public:
				std::array<vk::PipelineShaderStageCreateInfo, k_max_shader_stage_count> shader_stages{};

				std::array<vk::VertexInputBindingDescription, 16ull> binding_descriptions{};
				std::array<vk::VertexInputAttributeDescription, 16ull> attribute_descriptions{};
				vk::PipelineVertexInputStateCreateInfo vertex_input_state{};

				vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{};
				vk::PipelineTessellationStateCreateInfo tessellation_state{};

				std::array<vk::Viewport, 32ull> viewports{};
				std::array<vk::Rect2D, 32ull> scissors{};
				vk::PipelineViewportStateCreateInfo viewport_state{};

				vk::PipelineRasterizationStateCreateInfo rasterization_state{};
				vk::PipelineMultisampleStateCreateInfo multisample_state{};
				vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{};

				std::array<vk::PipelineColorBlendAttachmentState, k_max_render_target_count> color_blend_attachments{};
				vk::PipelineColorBlendStateCreateInfo color_blending_state{};

				std::array<vk::DynamicState, 32ull> dynamic_states{};
				vk::PipelineDynamicStateCreateInfo dynamic_state{};

				vk::PipelineRenderingCreateInfoKHR dynamic_rendering_create_info{};
				std::array<vk::Format, 8ull> color_attachments{};

				GraphicsPipelineBuilder();

				GraphicsPipelineBuilder& AddShaderStage(vk::PipelineShaderStageCreateInfo stage);

				// Vertex input state
				GraphicsPipelineBuilder& SetVertexInputStateFlags(vk::PipelineVertexInputStateCreateFlags flags);
				GraphicsPipelineBuilder& AddVertexInputBindingDescription(const vk::VertexInputBindingDescription& binding_description);
				GraphicsPipelineBuilder& AddVertexAttributeDescription(const vk::VertexInputAttributeDescription& attribute_description);

				// Input assembly state
				GraphicsPipelineBuilder& SetInputAsseblyStateFlags(vk::PipelineInputAssemblyStateCreateFlags flags);
				GraphicsPipelineBuilder& SetPrimitiveTopology(vk::PrimitiveTopology primitive_topology);
				GraphicsPipelineBuilder& SetPrimitiveRestartEnable(vk::Bool32 enable);

				// Tessellation state
				GraphicsPipelineBuilder& SetTessellationStateFlags(vk::PipelineTessellationStateCreateFlags flags);
				GraphicsPipelineBuilder& SetPatchControlPointsCount(uint32_t count);
					
				// Viewport state
				GraphicsPipelineBuilder& SetViewportStateFlags(vk::PipelineViewportStateCreateFlags flags);
				GraphicsPipelineBuilder& AddViewport(vk::Viewport viewport);
				GraphicsPipelineBuilder& AddScissorRect(vk::Rect2D rect);

				// Rasterization state
				GraphicsPipelineBuilder& SetRasterizationStateFlags(vk::PipelineRasterizationStateCreateFlags flags);
				GraphicsPipelineBuilder& SetDepthClampEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetRasterizerDiscardEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetPolygonMode(vk::PolygonMode mode);
				GraphicsPipelineBuilder& SetCullMode(vk::CullModeFlags flags);
				GraphicsPipelineBuilder& SetFrontFace(vk::FrontFace front_face);
				GraphicsPipelineBuilder& SetDepthBiasEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetDepthBiasConstantFactor(float factor);
				GraphicsPipelineBuilder& SetDepthBiasClamp(float clamp);
				GraphicsPipelineBuilder& SetDepthBiasSlopeFactor(float factor);
				GraphicsPipelineBuilder& SetLineWidth(float line_width);

				// Multisample state
				GraphicsPipelineBuilder& SetMultisampleStateFlags(vk::PipelineMultisampleStateCreateFlags flags);
				GraphicsPipelineBuilder& SetSampleCount(vk::SampleCountFlagBits sample_count);
				GraphicsPipelineBuilder& SetSampleShadingEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetMinSampleShading(float min_sample);
				GraphicsPipelineBuilder& SetSampleMask(const vk::SampleMask* mask);
				GraphicsPipelineBuilder& SetAlphaToCoverageEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetAlphaToOneEnable(vk::Bool32 enable);

				// Depth stencil state
				GraphicsPipelineBuilder& SetDepthStencilStateFlags(vk::PipelineDepthStencilStateCreateFlags flags);
				GraphicsPipelineBuilder& SetDepthTestEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetDepthWriteEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetDepthCompareOp(vk::CompareOp comapre_op);
				GraphicsPipelineBuilder& SetDepthBoundsTestEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetStencilTestEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetStencilOpFront(vk::StencilOpState front);
				GraphicsPipelineBuilder& SetStencilOpBack(vk::StencilOpState back);
				GraphicsPipelineBuilder& SetDepthBounds(float min_depth, float max_depth);

				// Color blending state
				GraphicsPipelineBuilder& SetColorBlendingStateFlags(vk::PipelineColorBlendStateCreateFlags flags);
				GraphicsPipelineBuilder& SetColorBlendingLogicOpEnable(vk::Bool32 enable);
				GraphicsPipelineBuilder& SetColorBlendingLogicOp(vk::LogicOp logic_op);
				GraphicsPipelineBuilder& AddColorBlendingAttachment(vk::PipelineColorBlendAttachmentState attachment);
				GraphicsPipelineBuilder& SetBlendConstants(std::array<float, 4> const& constants);

				// Dynamic state
				GraphicsPipelineBuilder& SetDynamicStateFlags(vk::PipelineDynamicStateCreateFlags flags);
				GraphicsPipelineBuilder& AddDynamicState(vk::DynamicState state);

				GraphicsPipelineBuilder& SetRenderPass(vk::RenderPass& render_pass);
				GraphicsPipelineBuilder SetSubpass(uint32_t subpass);

				GraphicsPipelineBuilder& AddColorAttachmentFormat(vk::Format format);
				GraphicsPipelineBuilder& SetDepthAttachmentFormat(vk::Format format);
				GraphicsPipelineBuilder& SetStencilAttachmentFormat(vk::Format format);

				GraphicsPipelineBuilder& EnableDynamicRendering(bool enable);

				uint32_t Hash() const;

				GraphicsPipeline Build(RenderDevice& render_device) const;
				std::unique_ptr<GraphicsPipeline> BuildUnique(RenderDevice& render_device) const;
			};

			class GraphicsPipeline : public Pipeline
			{
			public:
				GraphicsPipeline(GraphicsPipeline&&) = default;

				virtual ~GraphicsPipeline() = default;

				GraphicsPipeline(RenderDevice& render_device, const GraphicsPipelineBuilder& builder);
			};
		}
	}
}