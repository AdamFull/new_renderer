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

		protected:
			void _Execute(gfx::CommandList& command_list, float delta_time) override;

		private:
			gfx::RenderingInfo rendering_info{};
		};
	}
}