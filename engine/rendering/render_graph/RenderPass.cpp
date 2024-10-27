#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{
		RenderPass::RenderPass(gfx::Context& context, const RenderPassCreateInfo& create_info) :
			ShaderPass{ context, create_info.shader }
		{
			gfx::GraphicsPipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = create_info.pipeline_cache;
			pipeline_ci.pipeline_state = create_info.state;

			pipeline = context.CreatePipeline(pipeline_ci);
		}

		void RenderPass::_Execute(gfx::CommandList& command_list, float delta_time)
		{

		}
	}
}