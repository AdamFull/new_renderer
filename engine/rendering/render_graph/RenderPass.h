#pragma once

#include <engine/rendering/render_graph/ShaderPass.h>

namespace engine
{
	namespace render
	{
		class RenderPass : public ShaderPass
		{
		public:
			RenderPass(IRenderGraph& graph, const RenderPassCreateInfo& create_info);
			virtual ~RenderPass() = default;

			void AddColorAttachment(gfx::Attachment attachment);
			void SetDepthStencilAttachment(gfx::Attachment attachment);
			void SetDepthAttachment(gfx::Attachment attachment);
			void SetStencilAttachment(gfx::Attachment attachment);

			ShaderPassType GetPassType() override;

		protected:
			void UpdateRenderResolution();
			void _Execute(gfx::CommandList& command_list, float delta_time) override;

		private:
			gfx::RenderingInfo rendering_info{};
		};
	}
}