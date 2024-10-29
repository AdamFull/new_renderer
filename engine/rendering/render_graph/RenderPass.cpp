#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{
		RenderPass::RenderPass(gfx::Context& context, const RenderPassCreateInfo& create_info) :
			ShaderPass{ context, create_info }
		{
			gfx::GraphicsPipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = create_info.pipeline_cache;
			pipeline_ci.pipeline_state = create_info.state;

			pipeline = context.CreatePipeline(pipeline_ci);
		}

		void RenderPass::SetRenderResolution(uint32_t width, uint32_t height)
		{
			rendering_info.width = width;
			rendering_info.height = height;
		}

		void RenderPass::AddColorAttachment(gfx::Attachment attachment)
		{
			rendering_info.attachments[rendering_info.attachment_count++] = attachment;
		}

		void RenderPass::SetDepthStencilAttachment(gfx::Attachment attachment)
		{
			rendering_info.depth_attachment = attachment;
			rendering_info.stencil_attachment = attachment;
		}

		void RenderPass::SetDepthAttachment(gfx::Attachment attachment)
		{
			rendering_info.depth_attachment = attachment;
		}

		void RenderPass::SetStencilAttachment(gfx::Attachment attachment)
		{
			rendering_info.stencil_attachment = attachment;
		}

		ShaderPassType RenderPass::GetPassType()
		{
			return ShaderPassType::eGraphics;
		}

		void RenderPass::_Execute(gfx::CommandList& command_list, float delta_time)
		{
			command_list.BeginRendering(rendering_info);

			command_list.SetViewport(0.f, 0.f, rendering_info.width, rendering_info.height, 0.f, 1.f);
			command_list.SerScissor(0u, 0u, rendering_info.width, rendering_info.height);

			FlushResources(command_list);

			command_list.Draw(3u, 0u, 0u, 1u);

			command_list.EndRendering();

			// Clear rendering info
			rendering_info.attachment_count = 0;
		}
	}
}