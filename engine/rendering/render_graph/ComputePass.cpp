#include "ComputePass.h"

#include "../RendererInterface.h"
#include <edge/gfx/gfx_context.h>

namespace engine
{
	namespace render
	{
		ComputePass::ComputePass(ExecutionContext& execution_context, const ComputePassCreateInfo& create_info) :
			ShaderPass{ execution_context, create_info }
		{
			edge::gfx::ComputePipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader_effect->GetShader();
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = create_info.pipeline_cache;

			pipeline = context.CreatePipeline(pipeline_ci);
		}

		void ComputePass::_Execute(edge::gfx::CommandList& command_list, float delta_time)
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