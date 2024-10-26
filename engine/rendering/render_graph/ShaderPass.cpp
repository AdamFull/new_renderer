#include <engine/rendering/render_graph/ShaderPass.h>

#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{
		ShaderPass::ShaderPass(gfx::Context& context) : 
			context{ context }
		{

		}

		ShaderPass::~ShaderPass()
		{
			context.Destroy(pipeline);
			context.Destroy(root_signature);
			context.Destroy(shader);
		}

		void ShaderPass::SetExecutionContext(ExecutionContext* exec_context)
		{
			execution_context = exec_context;
		}

		void ShaderPass::Execute(float delta_time)
		{
			auto& command_list = execution_context->GetCurrentCommandList();
			command_list.BeginMarker(pass_label.c_str(), 0xFFFF11FF);

			// TODO: Also can add query here?

			_Execute(command_list, delta_time);

			command_list.EndMarker();
		}
	}
}