#pragma once

#include <engine/rendering/render_graph/ShaderPass.h>

namespace engine
{
	namespace render
	{
		class RenderPass : public ShaderPass
		{
		public:
			~RenderPass() = default;

		protected:
			void _Execute(gfx::CommandList& command_list, float delta_time) override;

		private:
			gfx::RenderingInfo rendering_info{};
		};
	}
}