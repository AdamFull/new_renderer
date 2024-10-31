#include <engine/rendering/render_graph/ShaderPass.h>

#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/gfx/Context.h>
#include <util/Logger.h>

#include <hash.hpp>

namespace engine
{
	namespace render
	{
		ShaderPass::ShaderPass(IRenderGraph& graph, const PassCreateInfo& create_info) :
			graph{ graph },
			context{ graph.GetContext() },
			shader{ create_info.shader },
			debug_name{ create_info.debug_name },
			one_time{ create_info.one_time },
			shared_descriptor{ create_info.shared_descriptor },
			draw_callback{ create_info.draw_callback }
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
			root_sign_ci.shared_descriptor = create_info.shared_descriptor;
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


		void ShaderPass::Evaluate(gfx::CommandList& command_list, float delta_time)
		{
			assert(execution_context && "CANNOT EXECUTE PASS WITHOUT EXECUTION CONTEXT");

			// Collect previous frame data
			CollectProfileData();

			command_list.BeginMarker(debug_name.c_str(), 0xFFFF11FF);

			// Reset performance counter
			command_list.ResetQuery(timestamp_query, 0u, 2u);
			command_list.BeginQuery(timestamp_query, 0u);

			// Bind pipeline
			command_list.BindPipeline(pipeline);

			_Execute(command_list, delta_time);

			command_list.EndQuery(timestamp_query, 1u);
			command_list.EndMarker();

			// Update one time flag for single time passes
			if (one_time)
			{
				can_execute = false;
			}
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
		
		void ShaderPass::FlushResources(gfx::CommandList& command_list)
		{
			if (!resource_descriptor)
			{
				auto& descriptor_allocator = execution_context->GetCurrentDescriptorAllocator();
				resource_descriptor = descriptor_allocator.AllocateDescriptors(root_signature);
			}

			// Update uniform buffers
			for (auto& handle : uniform_handles)
			{
				const auto& resource = handle.GetResource();

				// Request memory block for this uniform
				// TODO: may be request it inside uniform handler?
				auto uniform_block = execution_context->RequestUniformBlock(resource.size);
				uniform_block.Update(handle.Data());

				gfx::BufferDescriptor descriptor{};
				descriptor.type = resource.type;
				descriptor.set = resource.set;
				descriptor.binding = resource.binding;

				descriptor.buffer = &uniform_block.GetBuffer();
				descriptor.size = uniform_block.GetSize();
				descriptor.offset = uniform_block.GetOffset();

				resource_descriptor->BindBuffer(descriptor);
			}

			// Upload all resources
			resource_descriptor->Flush();

			gfx::ResourceDescriptor* resource_descriptors[]{ resource_descriptor, nullptr };
			uint32_t resource_descriptor_count{ 1u };

			if (shared_descriptor)
			{
				resource_descriptors[resource_descriptor_count++] = shared_descriptor->GetLatestDescriptors();
			}

			gfx::BindDescriptorsInfo bind_info{};
			bind_info.pipeline_type = pipeline->GetType();
			bind_info.resource_descriptors = resource_descriptors;
			bind_info.resource_descriptors_count = resource_descriptor_count;
			bind_info.root_signature = root_signature;

			command_list.BindDescriptors(bind_info);

			resource_descriptor = nullptr;
		}

		void ShaderPass::PushConstantRanges(gfx::CommandList& command_list)
		{
			auto& constant_data = constant_block->Data();
			command_list.PushConstants(shader, root_signature, std::span<const char>(constant_data.data(), constant_data.size()));
		}

		void ShaderPass::DivideWorkgroupByLocalSizes(uint32_t& group_x, uint32_t& group_y, uint32_t& group_z)
		{
			auto& reflection = shader->GetReflection();
			group_x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_x) / static_cast<float>(reflection.local_sizes[0]))), 1u);
			group_y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_y) / static_cast<float>(reflection.local_sizes[1]))), 1u);
			group_z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_z) / static_cast<float>(reflection.local_sizes[2]))), 1u);
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

			pass_delta_time = static_cast<float>(elapsed_time) * timestamp_frequency / 1000000000.f;
		}

		UniformHandle& ShaderPass::GetConstantBlock()
		{
			return *constant_block;
		}

		void ShaderPass::BindReadResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index)
		{

		}

		void ShaderPass::BindWriteResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index)
		{

		}

		void ShaderPass::BindResource(const ShaderResourceBinding& resource_binding)
		{
			if (!resource_descriptor)
			{
				auto& descriptor_allocator = execution_context->GetCurrentDescriptorAllocator();
				resource_descriptor = descriptor_allocator.AllocateDescriptors(root_signature);
			}

			auto& reflection = shader->GetReflection();
			// TODO: may be cache resource dst from recursion for lookup only one time
			// Check that we have this binding point
			auto found = std::find_if(reflection.shader_resources.Begin(), reflection.shader_resources.End(), [&resource_binding](const gfx::ShaderResource& resource) { return resource.name_hash == resource_binding.binding_point; });
			if (found != reflection.shader_resources.End())
			{
				const gfx::ShaderResource& resource = *found;

				if (resource_binding.buffer)
				{
					gfx::BufferDescriptor descriptor{};
					descriptor.type = resource.type;
					descriptor.set = resource.set;
					descriptor.binding = resource.binding;

					descriptor.buffer = resource_binding.buffer;
					descriptor.size = resource_binding.buffer->GetSize();

					resource_descriptor->BindBuffer(descriptor);
				}
				else if (resource_binding.texture)
				{
					gfx::TextureDescriptor descriptor{};
					descriptor.type = resource.type;
					descriptor.set = resource.set;
					descriptor.binding = resource.binding;

					descriptor.texture = resource_binding.texture;
					descriptor.sampler = resource_binding.sampler;
					descriptor.subresource_index = resource_binding.subresource;

					resource_descriptor->BindTexture(descriptor);
				}
				else
				{
					log_warning("No resources provided for bind");
					assert(false && "NO BIND RESOURCE PROVIDED");
					return;
				}
				return;
			}

			log_warning("Resource binding was not found for this pass.");
			assert(false && "RESOURCE BINDING NOT FOUND");
		}

		bool ShaderPass::CanExecute() const
		{
			return can_execute;
		}
	}
}