#include <engine/rendering/render_graph/ShaderPass.h>

#include <engine/rendering/gfx/Context.h>

#include <hash.hpp>

namespace engine
{
	namespace render
	{
		ShaderPass::ShaderPass(gfx::Context& context, gfx::Shader* shader) :
			context{ context },
			shader{ shader }
		{
			auto& reflection = shader->GetReflection();

			// Create uniform block handlers for resources
			for (auto it = reflection.shader_resources.Begin(); it != reflection.shader_resources.End(); ++it)
			{
				if (it->type == gfx::DescriptorType::eUniformBuffer)
				{
					uniform_handles.emplace_back(reflection, (*it));
				}
			}

			// Also add push constant block
			constant_block = std::make_unique<UniformHandle>(reflection, reflection.constants);

			// Create root signature
			gfx::RootSignatureCreateInfo root_sign_ci{};
			root_sign_ci.shader = shader;
			// TODO: also load immutable shaders from pass info or from somewhere another?

			root_signature = context.CreateRootSignature(root_sign_ci);
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

		UniformHandle& ShaderPass::GetUniformBlock(uint32_t name_hash)
		{
			auto found = std::find_if(uniform_handles.begin(), uniform_handles.end(), [&name_hash](const UniformHandle& handle) { return handle.GetNameHash() == name_hash; });
			if (found != uniform_handles.end())
				return *found;

			assert(false && "Requested uniform block is not exists");

			return *constant_block;
		}

		UniformHandle& ShaderPass::GetUniformBlock(const char* name)
		{
			return GetUniformBlock(utl::murmur_hash(name));
		}

		UniformHandle ShaderPass::GetConstantBlock()
		{
			return *constant_block;
		}
	}
}