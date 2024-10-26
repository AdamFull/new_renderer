#include <engine/rendering/render_graph/ComputePass.h>

namespace engine
{
	namespace render
	{
		ComputePass::ComputePass(gfx::Context& context, gfx::Shader* shader) :
			ShaderPass{ context, shader }
		{

		}

		void ComputePass::_Execute(gfx::CommandList& command_list, float delta_time)
		{

		}
	}
}