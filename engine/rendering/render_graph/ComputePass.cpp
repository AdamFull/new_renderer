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
		}

		void ComputePass::_Execute(edge::gfx::ICommandList& command_list, float delta_time)
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