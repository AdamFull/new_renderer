#include <engine/rendering/render_graph/RenderPass.h>

namespace engine
{
	namespace render
	{
		RenderPass::RenderPass(gfx::Context& context, gfx::Shader* shader) :
			ShaderPass{ context, shader }
		{

		}

		void RenderPass::_Execute(gfx::CommandList& command_list, float delta_time)
		{

		}
	}
}