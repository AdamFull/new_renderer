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

			// Prepare barriers
			resource_barriers.buffer_barriers = buffer_barriers.data();
			resource_barriers.texture_barriers = texture_barriers.data();
			resource_barriers.memory_barriers = memory_barriers.data();
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

			if (!can_execute)
				return;

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


			// Flush resource barriers
			command_list.ResourceBarrier(resource_barriers);
			resource_barriers.buffer_barrier_count = 0;
			resource_barriers.texture_barrier_count = 0;
			resource_barriers.memory_barrier_count = 0;
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

		void ShaderPass::BindResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index, gfx::AccessTypeFlags access)
		{
			if (!resource_descriptor)
			{
				auto& descriptor_allocator = execution_context->GetCurrentDescriptorAllocator();
				resource_descriptor = descriptor_allocator.AllocateDescriptors(root_signature);
			}

			auto& reflection = shader->GetReflection();

			auto found = std::find_if(reflection.shader_resources.Begin(), reflection.shader_resources.End(), [&binding_point](const gfx::ShaderResource& resource) { return resource.name_hash == binding_point; });
			if (found != reflection.shader_resources.End())
			{
				auto& render_resource = graph.GetResource(resource_index);

				bool skip_barriers{ false };
				if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
				{
					skip_barriers = true;
				}

				auto last_access = render_resource.GetAccess(subresource_index);


				if ((last_access & gfx::AccessTypeFlag::eRead) && (access & gfx::AccessTypeFlag::eRead))
					skip_barriers = true;

				// Generate barriers if needed
				if (!skip_barriers)
				{
					bool is_initial_state = last_access == 0;
					bool is_read_after_write = (last_access & gfx::AccessTypeFlag::eWrite) && (access & gfx::AccessTypeFlag::eRead);
					bool is_write_after_write = (last_access & gfx::AccessTypeFlag::eWrite) && (access & gfx::AccessTypeFlag::eWrite);
					bool is_write_after_read = (last_access & gfx::AccessTypeFlag::eRead) && (access & gfx::AccessTypeFlag::eWrite);

					gfx::ResourceStateFlags final_state{};
					if (GetPassType() == ShaderPassType::eCompute)
						final_state = render_resource.final_state_comp;
					else
						final_state = render_resource.final_state_other;

					gfx::ResourceStateFlags new_state{};
					if (is_initial_state)
						new_state = (access & gfx::AccessTypeFlag::eRead) ? final_state : render_resource.intermediate_state;
					else if (is_read_after_write)
						new_state = final_state;
					else if (is_write_after_read)
						new_state = render_resource.intermediate_state;
					else
						new_state = final_state;

					if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
					{
						auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
						barrier.texture = graph.GetTexture(render_resource);
						barrier.src_state = render_resource.GetState(subresource_index);
						barrier.dst_state = new_state;
					}
					else if (render_resource.resource_type & RenderResourceTypeFlag::eBuffer)
					{
						// At first creating resource barrier
						auto& barrier = buffer_barriers[resource_barriers.buffer_barrier_count++];
						barrier.buffer = graph.GetBuffer(render_resource);
						barrier.src_state = render_resource.GetState();
						barrier.dst_state = new_state;
					}

					// Update resource state
					render_resource.SetState(new_state);
					render_resource.SetAccess(access, subresource_index);
				}

				// Bind resources
				const gfx::ShaderResource& refl_resource = *found;
				if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
				{
					// Create texture descriptor
					gfx::TextureDescriptor descriptor{};
					descriptor.type = refl_resource.type;
					descriptor.set = refl_resource.set;
					descriptor.binding = refl_resource.binding;

					descriptor.texture = graph.GetTexture(render_resource);
					descriptor.sampler = graph.GetSampler(render_resource);
					descriptor.subresource_index = subresource_index;
					descriptor.resource_state = render_resource.GetState(subresource_index);

					resource_descriptor->BindTexture(descriptor);
				}
				else if (render_resource.resource_type & RenderResourceTypeFlag::eBuffer)
				{
					// At first creating resource barrier
					auto& barrier = buffer_barriers[resource_barriers.buffer_barrier_count++];
					barrier.buffer = graph.GetBuffer(render_resource);
					barrier.src_state = render_resource.GetState();
					barrier.dst_state = render_resource.GetState();

					// And also create resource descriptor
					gfx::BufferDescriptor descriptor{};
					descriptor.type = refl_resource.type;
					descriptor.set = refl_resource.set;
					descriptor.binding = refl_resource.binding;

					descriptor.buffer = barrier.buffer;
					descriptor.size = descriptor.buffer->GetSize();

					resource_descriptor->BindBuffer(descriptor);
				}
				else
				{
					assert(false && "UNKNOWN RESOURCE TYPE");
				}
			}
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
			BindResource(binding_point, render_resource_id, subresource_index, gfx::AccessTypeFlag::eRead);
		}

		void ShaderPass::BindWriteResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index)
		{
			BindResource(binding_point, render_resource_id, subresource_index, gfx::AccessTypeFlag::eWrite);
		}

		bool ShaderPass::CanExecute() const
		{
			return can_execute;
		}
	}
}