#include <engine/rendering/render_graph/ComputePass.h>

#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{
		ComputePass::ComputePass(gfx::Context& context, const ComputePassCreateInfo& create_info) :
			ShaderPass{ context, create_info }
		{
			gfx::ComputePipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = create_info.pipeline_cache;

			pipeline = context.CreatePipeline(pipeline_ci);
		}

		void ComputePass::_Execute(gfx::CommandList& command_list, float delta_time)
		{
			if (draw_callback)
			{
				draw_callback(*this, command_list, delta_time);
			}
		}

		ShaderPassType ComputePass::GetPassType()
		{
			return ShaderPassType::eCompute;
		}
	}
}