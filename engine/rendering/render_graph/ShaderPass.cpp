#include <engine/rendering/render_graph/ShaderPass.h>

#include <engine/rendering/gfx/Context.h>
#include <util/Logger.h>

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
			// TODO: also load immutable samplers from pass info or from somewhere another?

			root_signature = context.CreateRootSignature(root_sign_ci);

			// Create timestamp query 
			gfx::QueryPoolCreateInfo query_pool_ci{};
			query_pool_ci.query_count = 2u;
			query_pool_ci.type = gfx::QueryType::eTimestamp;

			timestamp_query = context.CreateQueryPool(query_pool_ci);
			timestamp_query->Reset(0u);

			context.RequestTimestampFrequency(&timestamp_frequency);
		}

		ShaderPass::~ShaderPass()
		{
			context.Destroy(pipeline);
			context.Destroy(root_signature);
			context.Destroy(shader);
			context.Destroy(timestamp_query);
		}

		void ShaderPass::SetExecutionContext(ExecutionContext* exec_context)
		{
			execution_context = exec_context;
		}


		void ShaderPass::Execute(float delta_time)
		{
			// Collect previous frame data
			CollectProfileData();

			auto& command_list = execution_context->GetCurrentCommandList();
			command_list.BeginMarker(pass_label.c_str(), 0xFFFF11FF);

			// Reset performance counter
			//timestamp_query->Reset(0u);
			command_list.ResetQuery(timestamp_query, 0u, 2u);
			command_list.WriteTimestamp(timestamp_query, 0u, gfx::StageFlag::eTopOfPipe);

			// Bind pipeline
			command_list.BindPipeline(pipeline);

			_Execute(command_list, delta_time);

			command_list.WriteTimestamp(timestamp_query, 1u, gfx::StageFlag::eBottomOfPipe);
			command_list.EndMarker();

			// Reset resource bindings (idk, may be don't needed in future)
			resource_bindings.clear();
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

		void ShaderPass::PushConstantRanges(gfx::CommandList& command_list)
		{
			auto& constant_data = constant_block->Data();
			command_list.PushConstants(shader, root_signature, std::span<const char>(constant_data.data(), constant_data.size()));
		}

		void ShaderPass::CollectProfileData()
		{
			uint64_t timestamps[2] = {};
			timestamp_query->GetData(0u, &timestamps[0]);

			uint64_t elapsed_time = timestamps[1] - timestamps[0];
			if (timestamps[1] <= timestamps[0])
			{
				elapsed_time = 0ull;
			}

			execution_time = elapsed_time;

			pass_delta_time = static_cast<double>(elapsed_time) * timestamp_frequency / 1000000000.0;
		}

		UniformHandle ShaderPass::GetConstantBlock()
		{
			return *constant_block;
		}

		void ShaderPass::BindResource(const ShaderResourceBinding& resource_binding)
		{
			auto& reflection = shader->GetReflection();
			// TODO: may be cache resource dst from recursion for lookup only one time
			// Check that we have this binding point
			auto found = std::find_if(reflection.shader_resources.Begin(), reflection.shader_resources.End(), [&resource_binding](const gfx::ShaderResource& resource) { return resource.name_hash == resource_binding.binding_point; });
			if (found == reflection.shader_resources.End())
			{
				log_warning("Resource binding was not found for this pass.");
				assert(false && "RESOURCE BINDING NOT FOUND");
				return;
			}

			resource_bindings.push_back(resource_binding);
		}
	}
}