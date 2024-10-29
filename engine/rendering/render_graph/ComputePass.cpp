#include <engine/rendering/render_graph/ComputePass.h>

#include <engine/rendering/gfx/Context.h>

namespace engine
{
	namespace render
	{
		ComputePass::ComputePass(gfx::Context& context, const ComputePassCreateInfo& create_info) :
			ShaderPass{ context, create_info },
			texture_size_target{create_info.texture_size_target },
			texture_subresource_index{ create_info.texture_subresource_index },
			buffer_size_target{ create_info.buffer_size_target },
			divide_by_locals{create_info.divide_by_locals }
		{
			gfx::ComputePipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = create_info.pipeline_cache;

			pipeline = context.CreatePipeline(pipeline_ci);
		}

		void ComputePass::_Execute(gfx::CommandList& command_list, float delta_time)
		{
			FlushResources(command_list);

			uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

			if (texture_size_target)
			{
				group_x = texture_size_target->GetWidth();
				group_y = texture_size_target->GetHeight();
			}
			else if (buffer_size_target)
			{

			}

			if (divide_by_locals)
			{
				auto& reflection = shader->GetReflection(); 
				group_x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_x) / static_cast<float>(reflection.local_sizes[0]))), 1u);
				group_y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_y) / static_cast<float>(reflection.local_sizes[1]))), 1u);
				group_z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_z) / static_cast<float>(reflection.local_sizes[2]))), 1u);
			}

			command_list.Dispatch(group_x, group_y, group_z);
		}

		ShaderPassType ComputePass::GetPassType()
		{
			return ShaderPassType::eCompute;
		}
	}
}