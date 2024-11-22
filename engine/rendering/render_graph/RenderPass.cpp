#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/gfx/Context.h>

#include <util/Logger.h>

#include <tracy/Tracy.hpp>

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
			pipeline_ci.pipeline_state = create_info.pipeline_state;

			for (auto& attachment : resource_attachments)
			{
				auto render_target = graph.GetTexture(attachment.render_resource_id);
				auto rt_format = render_target->GetFormat();
				if (attachment.flags & ResourceAttachmentTypeFlag::eDepth)
					pipeline_ci.depth_format = rt_format;
				else if (attachment.flags & ResourceAttachmentTypeFlag::eStencil)
					pipeline_ci.stencil_format = rt_format;
				else
				{
					pipeline_ci.color_formats[pipeline_ci.color_format_count++] = rt_format;
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

		void RenderPass::_Execute(gfx::CommandList& command_list, float delta_time)
		{
			ZoneScoped;

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

				gfx::ResourceStateFlags new_state{ gfx::ResourceStateFlag::eWrite };

				auto last_access = render_resource.GetAccess();

				gfx::Attachment* attachment{ nullptr };
				if (resource_attachment.flags & ResourceAttachmentTypeFlag::eDepth)
				{
					attachment = &rendering_info.depth_attachment;
					new_state |= gfx::ResourceStateFlag::eDepthAttachment;
				}
				else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eStencil)
				{
					attachment = &rendering_info.stencil_attachment;
					new_state |= gfx::ResourceStateFlag::eStencilAttachment;
				}
				else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eColor)
				{
					new_state |= gfx::ResourceStateFlag::eColorAttachment;
					attachment = &rendering_info.attachments[rendering_info.attachment_count++];
				}

				auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
				barrier.texture = graph.GetTexture(render_resource);
				barrier.src_state = render_resource.GetState();
				barrier.dst_state = new_state;

				if (last_access == 0)
				{
					attachment->load_op = gfx::LoadOp::eClear;
				}
				else
				{
					attachment->load_op = gfx::LoadOp::eLoad;
					new_state |= gfx::ResourceStateFlag::eRead;
				}

				attachment->texture = barrier.texture;
				attachment->store_op = gfx::StoreOp::eNone;
				attachment->texture_state = barrier.dst_state;
				attachment->clear_color = resource_attachment.clear_color;

				rendering_info.width = attachment->texture->GetWidth();
				rendering_info.height = attachment->texture->GetHeight();

				rendering_info.layer_count = attachment->texture->GetArrayLayerCount();

				render_resource.SetState(barrier.dst_state);
			}

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