#include "RenderPass.h"

#include "../RendererInterface.h"
#include <edge/gfx/gfx_context.h>

#include <edge/util/Logger.h>

#include <edge/util/utl_profiler.h>

namespace engine
{
	namespace render
	{
		RenderPass::RenderPass(ExecutionContext& execution_context, const RenderPassCreateInfo& create_info) :
			ShaderPass{ execution_context, create_info },
			resource_attachments{ create_info.resource_attachments }
		{
		}

		void RenderPass::AddColorAttachment(edge::gfx::Attachment attachment)
		{
			rendering_info.attachments[rendering_info.attachment_count++] = attachment;
		}

		ShaderPassType RenderPass::GetPassType()
		{
			return ShaderPassType::eGraphics;
		}

		void RenderPass::Begin(edge::gfx::ICommandList& command_list)
		{
			command_list.begin_rendering(rendering_info);
		}

		void RenderPass::End(edge::gfx::ICommandList& command_list)
		{
			command_list.end_rendering();
		}

		void RenderPass::_Execute(edge::gfx::ICommandList& command_list, float delta_time)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			// TODO: cache it???
			for (auto& resource_attachment : resource_attachments)
			{
				auto& render_resource = execution_context.GetResource(resource_attachment.render_resource_id);

				// Wtf? Trying write to static resource????
				if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
				{
					EDGE_LOGE("Trying bind static texture as color attachment!");
					assert(false && "INCORRECT OUTPUT RESOURCE BINDING!");
					continue;
				}

				edge::gfx::ResourceStateFlags new_state{ edge::gfx::ResourceStateFlag::eWrite };

				auto last_access = render_resource.GetAccess();

				edge::gfx::Attachment* attachment{ nullptr };
				if (resource_attachment.flags & ResourceAttachmentTypeFlag::eDepth)
				{
					attachment = &rendering_info.depth_attachment;
					new_state |= edge::gfx::ResourceStateFlag::eDepthAttachment;
				}
				else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eStencil)
				{
					attachment = &rendering_info.stencil_attachment;
					new_state |= edge::gfx::ResourceStateFlag::eStencilAttachment;
				}
				else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eColor)
				{
					new_state |= edge::gfx::ResourceStateFlag::eColorAttachment;
					attachment = &rendering_info.attachments[rendering_info.attachment_count++];
				}

				auto& barrier = texture_barriers.emplace_back();
				barrier.texture = execution_context.GetTexture(render_resource);
				barrier.src_state = render_resource.GetState();
				barrier.dst_state = new_state;

				if (last_access == 0)
				{
					attachment->load_op = edge::gfx::LoadOp::eClear;
				}
				else
				{
					attachment->load_op = edge::gfx::LoadOp::eLoad;
					new_state |= edge::gfx::ResourceStateFlag::eRead;
				}

				attachment->texture = barrier.texture;
				attachment->store_op = edge::gfx::StoreOp::eNone;
				attachment->texture_state = barrier.dst_state;
				attachment->clear_color = resource_attachment.clear_color;

				rendering_info.width = attachment->texture->get_width();
				rendering_info.height = attachment->texture->get_height();

				rendering_info.layer_count = attachment->texture->get_array_layer_count();

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