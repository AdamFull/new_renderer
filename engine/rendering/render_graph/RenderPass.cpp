#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{
		RenderPass::RenderPass(IRenderGraph& graph, const RenderPassCreateInfo& create_info) :
			ShaderPass{ graph, create_info }
		{
			gfx::GraphicsPipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = create_info.pipeline_cache;
			pipeline_ci.pipeline_state = create_info.state;

			pipeline = context.CreatePipeline(pipeline_ci);
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

		void RenderPass::UpdateRenderResolution()
		{
			// Try find render resolution from any attachment
			for (int32_t i = 0; i < rendering_info.attachment_count; ++i)
			{
				auto& attachment = rendering_info.attachments[i];
				rendering_info.width = attachment.texture->GetWidth();
				rendering_info.height = attachment.texture->GetHeight();
				return;
			}

			// Or from depth attachment
			if (rendering_info.depth_attachment.texture)
			{
				auto& attachment = rendering_info.depth_attachment;
				rendering_info.width = attachment.texture->GetWidth();
				rendering_info.height = attachment.texture->GetHeight();
				return;
			}

			// Or from stencil one
			if (rendering_info.stencil_attachment.texture)
			{
				auto& attachment = rendering_info.stencil_attachment;
				rendering_info.width = attachment.texture->GetWidth();
				rendering_info.height = attachment.texture->GetHeight();
				return;
			}
		}

		void RenderPass::_Execute(gfx::CommandList& command_list, float delta_time)
		{
			UpdateRenderResolution();

			command_list.BeginRendering(rendering_info);

			if (draw_callback)
			{
				draw_callback(*this, command_list, delta_time);
			}

			command_list.EndRendering();

			// Clear rendering info
			rendering_info.attachment_count = 0;
			rendering_info.depth_attachment = {};
			rendering_info.stencil_attachment = {};
		}
	}
}