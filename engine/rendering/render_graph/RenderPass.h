#pragma once

#include <engine/rendering/render_graph/ShaderPass.h>

namespace engine
{
	namespace render
	{
		class RenderPass : public ShaderPass
		{
		public:
			RenderPass(gfx::Context& context, const RenderPassCreateInfo& create_info);
			~RenderPass() = default;

			void SetRenderResolution(uint32_t width, uint32_t height);

			void AddColorAttachment(gfx::Attachment attachment);
			void SetDepthStencilAttachment(gfx::Attachment attachment);
			void SetDepthAttachment(gfx::Attachment attachment);
			void SetStencilAttachment(gfx::Attachment attachment);

			ShaderPassType GetPassType() override;

		protected:
			void _Execute(gfx::CommandList& command_list, float delta_time) override;

		private:
			gfx::RenderingInfo rendering_info{};
		};
	}
}