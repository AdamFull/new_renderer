#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/gfx/Context.h>

#include <util/Logger.h>

namespace engine
{
	namespace render
	{
		RenderPass::RenderPass(IRenderGraph& graph, const RenderPassCreateInfo& create_info) :
			ShaderPass{ graph, create_info },
			resource_attachments{ create_info.resource_attachments }
		{
			gfx::GraphicsPipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = create_info.pipeline_cache;
			//pipeline_ci.pipeline_state = create_info.state;

			pipeline_ci.pipeline_state.vertex_layout = create_info.vertex_layout;
			pipeline_ci.pipeline_state.color_blending_state = create_info.color_blending_state;
			pipeline_ci.pipeline_state.primitive_topology = create_info.primitive_topology;

			for (auto& attachment : resource_attachments)
			{
				auto render_target = graph.GetTexture(attachment.render_resource_id);
				auto rt_format = render_target->GetFormat();
				if (attachment.flags & ResourceAttachmentTypeFlag::eDepth)
					pipeline_ci.pipeline_state.depth_format = rt_format;
				else if (attachment.flags & ResourceAttachmentTypeFlag::eStencil)
					pipeline_ci.pipeline_state.stencil_format = rt_format;
				else
				{
					pipeline_ci.pipeline_state.color_formats[pipeline_ci.pipeline_state.color_format_count++] = rt_format;
				}
			}

			pipeline = context.CreatePipeline(pipeline_ci);
		}

		void RenderPass::AddColorAttachment(gfx::Attachment attachment)
		{
			rendering_info.attachments[rendering_info.attachment_count++] = attachment;
		}

		ShaderPassType RenderPass::GetPassType()
		{
			return ShaderPassType::eGraphics;
		}

		void RenderPass::Begin(gfx::CommandList& command_list)
		{
			command_list.BeginRendering(rendering_info);
		}

		void RenderPass::End(gfx::CommandList& command_list)
		{
			command_list.EndRendering();
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
			// TODO: cache it???
			for (auto& resource_attachment : resource_attachments)
			{
				auto& render_resource = graph.GetResource(resource_attachment.render_resource_id);

				// Wtf? Trying write to static resource????
				if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
				{
					log_error("Trying bind static texture as color attachment!");
					assert(false && "INCORRECT OUTPUT RESOURCE BINDING!");
					continue;
				}

				auto last_access = render_resource.GetAccess();

				auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
				barrier.texture = graph.GetTexture(render_resource);
				barrier.src_state = render_resource.GetState();
				barrier.dst_state = render_resource.intermediate_state;

				gfx::Attachment* attachment{ nullptr };
				if (resource_attachment.flags & ResourceAttachmentTypeFlag::eDepth)
					attachment = &rendering_info.depth_attachment;
				else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eStencil)
					attachment = &rendering_info.stencil_attachment;
				else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eColor)
					attachment = &rendering_info.attachments[rendering_info.attachment_count++];

				attachment->texture = graph.GetTexture(resource_attachment.render_resource_id);
				attachment->load_op = last_access == 0 ? gfx::LoadOp::eClear : gfx::LoadOp::eLoad;
				attachment->store_op = gfx::StoreOp::eNone;
				attachment->texture_state = barrier.dst_state;
				attachment->clear_color = resource_attachment.clear_color;

				render_resource.SetState(barrier.dst_state);
			}

			UpdateRenderResolution();

			if (draw_callback)
			{
				draw_callback(*this, command_list, delta_time);
			}

			// Clear rendering info
			rendering_info.attachment_count = 0;
			rendering_info.depth_attachment = {};
			rendering_info.stencil_attachment = {};
		}
	}
}