#include "ShaderPass.h"

#include "../RendererInterface.h"
#include <edge/gfx/gfx_context.h>
#include <edge/gfx/gfx_util.h>

#include <edge/util/Logger.h>
#include <edge/util/Hash.hpp>

#include <edge/util/Profiler.h>

namespace engine
{
	namespace render
	{
		ShaderPass::ShaderPass(ExecutionContext& execution_context, const PassCreateInfo& create_info) :
			debug_name{ create_info.name },
			shader{ create_info.shader },
			root_signature{ create_info.root_signature },
			pipeline{ create_info.pipeline },
			shared_descriptor{ create_info.shared_descriptor },
			execution_context{ execution_context },
			context{ execution_context.GetContext() },
			one_time{ create_info.one_time },
			draw_callback{ create_info.draw_callback },
			enabled_callback{ create_info.enabled_callback }
		{
			auto& reflection = shader->get_reflection();
			
			// Create uniform block handlers for resources
			for (auto it = reflection.shader_resources.begin(); it != reflection.shader_resources.end(); ++it)
			{
				if (it->type == edge::gfx::DescriptorType::eUniformBuffer)
				{
					uniform_handles.emplace_back(reflection, (*it));
				}
			}

			// Also add push constant block
			constant_block = std::make_unique<UniformHandle>(reflection, reflection.constants);

			// Create timestamp query 
			edge::gfx::QueryPoolCreateInfo query_pool_ci{};
			query_pool_ci.query_count = 2u;
			query_pool_ci.type = edge::gfx::QueryType::eTimestamp;

			timestamp_query = context.create_query_pool(query_pool_ci);
			timestamp_query->reset(0u);

			context.request_timestamp_frequency(&timestamp_frequency);

			// Prepare barriers
			resource_barriers.buffer_barriers = buffer_barriers.data();
			resource_barriers.texture_barriers = texture_barriers.data();
			resource_barriers.memory_barriers = memory_barriers.data();
		}

		ShaderPass::~ShaderPass()
		{
		}


		void ShaderPass::Execute(edge::gfx::ICommandList& command_list, float delta_time)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			if (!can_execute)
				return;

			if (enabled_callback && !enabled_callback())
				return;

			// Collect previous frame data
			CollectProfileData();

			command_list.begin_marker(debug_name.c_str(), 0xFFFF11FF);

			// Reset performance counter
			command_list.reset_query(timestamp_query, 0u, 2u);
			command_list.begin_query(timestamp_query, 0u);

			// Bind pipeline
			command_list.bind_pipeline(pipeline);

			_Execute(command_list, delta_time);

			command_list.end_query(timestamp_query, 1u);
			command_list.end_marker();

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
			return GetUniformBlock(edge::murmur_hash(name));
		}
		
		void ShaderPass::BindResources(edge::gfx::ICommandList& command_list)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			if (!resource_descriptor)
			{
				auto& descriptor_allocator = execution_context.GetCurrentDescriptorAllocator();
				resource_descriptor = descriptor_allocator.allocate_descriptors(root_signature);
			}

			// Update uniform buffers
			for (auto& handle : uniform_handles)
			{
				const auto& resource = handle.GetResource();

				// Request memory block for this uniform
				// TODO: may be request it inside uniform handler?
				auto uniform_block = execution_context.RequestUniformBlock(resource.size);
				uniform_block.update(std::span(handle.Data()));

				edge::gfx::BufferDescriptor descriptor{};
				descriptor.type = resource.type;
				descriptor.set = resource.set;
				descriptor.binding = resource.binding;

				descriptor.buffer = &uniform_block.GetBuffer();
				descriptor.size = uniform_block.GetSize();
				descriptor.offset = uniform_block.GetOffset();

				resource_descriptor->bind_buffer(descriptor);
			}

			// Upload all resources
			resource_descriptor->flush();

			edge::gfx::IResourceDescriptor* resource_descriptors[]{ resource_descriptor, nullptr };
			uint32_t resource_descriptor_count{ 1u };

			if (shared_descriptor)
			{
				resource_descriptors[resource_descriptor_count++] = shared_descriptor->get_latest_descriptors();
			}

			edge::gfx::BindDescriptorsInfo bind_info{};
			bind_info.pipeline_type = pipeline->get_type();
			bind_info.resource_descriptors = { resource_descriptors, resource_descriptor_count };
			bind_info.root_signature = root_signature;

			command_list.bind_descriptors(bind_info);

			resource_descriptor = nullptr;
		}

		void ShaderPass::PushConstantRanges(edge::gfx::ICommandList& command_list)
		{
			auto& constant_data = constant_block->Data();
			command_list.push_constants(shader, root_signature, std::span<const char>(constant_data.data(), constant_data.size()));
		}

		void ShaderPass::DivideWorkgroupByLocalSizes(uint32_t& group_x, uint32_t& group_y, uint32_t& group_z)
		{
			auto& reflection = shader->get_reflection();
			group_x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_x) / static_cast<float>(reflection.local_sizes[0]))), 1u);
			group_y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_y) / static_cast<float>(reflection.local_sizes[1]))), 1u);
			group_z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_z) / static_cast<float>(reflection.local_sizes[2]))), 1u);
		}

		void ShaderPass::ResourceDependency(uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto& render_resource = execution_context.GetResource(resource_id);

			edge::gfx::ResourceStateFlags new_state = edge::gfx::get_access_state(access);

			auto pass_type = GetPassType();
			if (pass_type == ShaderPassType::eCompute)
				new_state |= edge::gfx::ResourceStateFlag::eCompute;
			else if (pass_type == ShaderPassType::eGraphics)
				new_state |= edge::gfx::ResourceStateFlag::eGraphics;

			if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				// May be create another tag like unordered resource?
				if (render_resource.resource_type & RenderResourceTypeFlag::eStorageTarget)
					new_state |= edge::gfx::ResourceStateFlag::eGeneral;

				auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
				barrier.texture = execution_context.GetTexture(render_resource);
				barrier.subresource_index = sebresource_id;
				barrier.src_state = render_resource.GetState();
				barrier.dst_state = new_state;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer)
			{
				if(render_resource.resource_type & RenderResourceTypeFlag::eStorageBuffer)
					new_state |= edge::gfx::ResourceStateFlag::eStorage;
				
				if (render_resource.resource_type & RenderResourceTypeFlag::eIndirectBuffer)
					new_state |= edge::gfx::ResourceStateFlag::eIndirect;

				auto& barrier = buffer_barriers[resource_barriers.buffer_barrier_count++];
				barrier.buffer = execution_context.GetBuffer(render_resource);
				barrier.src_state = render_resource.GetState();
				barrier.dst_state = new_state;
			}

			render_resource.SetState(new_state);
		}

		void ShaderPass::ResourceDependency(uint32_t binding_point, uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			auto& reflection = shader->get_reflection();
			auto found = std::find_if(reflection.shader_resources.begin(), reflection.shader_resources.end(), 
				[&binding_point](const edge::gfx::ShaderResource& resource) 
				{ return resource.name_hash == binding_point; });

			if (found != reflection.shader_resources.end())
			{
				auto& render_resource = execution_context.GetResource(resource_id);

				edge::gfx::ResourceStateFlags new_state = edge::gfx::get_access_state(access);

				auto pass_type = GetPassType();
				if (pass_type == ShaderPassType::eCompute)
					new_state |= edge::gfx::ResourceStateFlag::eCompute;
				else if (pass_type == ShaderPassType::eGraphics)
					new_state |= edge::gfx::ResourceStateFlag::eGraphics;

				const edge::gfx::ShaderResource& refl_resource = *found;
				new_state |= edge::gfx::get_state_by_descriptor_type(refl_resource.type);

				if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
				{
					// May be create another tag like unordered resource?
					if(render_resource.resource_type & RenderResourceTypeFlag::eStorageTarget)
						new_state |= edge::gfx::ResourceStateFlag::eGeneral;

					auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
					barrier.texture = execution_context.GetTexture(render_resource);
					barrier.subresource_index = sebresource_id;
					barrier.src_state = render_resource.GetState();
					barrier.dst_state = new_state;
				}
				else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer)
				{
					auto& barrier = buffer_barriers[resource_barriers.buffer_barrier_count++];
					barrier.buffer = execution_context.GetBuffer(render_resource);
					barrier.src_state = render_resource.GetState();
					barrier.dst_state = new_state;
				}

				render_resource.SetState(new_state);
			}
		}

		void ShaderPass::CollectProfileData()
		{
			uint64_t timestamps[2] = {};
			timestamp_query->get_data(0u, &timestamps[0]);

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

		void ShaderPass::ReadResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index)
		{
			ResourceDependency(binding_point, render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eRead);
		}

		void ShaderPass::ReadResource(uint32_t render_resource_id, uint32_t subresource_index)
		{
			ResourceDependency(render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eRead);
		}

		void ShaderPass::WriteResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index)
		{
			ResourceDependency(binding_point, render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eWrite);
		}

		void ShaderPass::WriteResource(uint32_t render_resource_id, uint32_t subresource_index)
		{
			ResourceDependency(render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eWrite);
		}

		void ShaderPass::ResolveDependencies(edge::gfx::ICommandList& command_list)
		{
			// Flush resource barriers
			command_list.resource_barrier(resource_barriers);
			resource_barriers.buffer_barrier_count = 0;
			resource_barriers.texture_barrier_count = 0;
			resource_barriers.memory_barrier_count = 0;
		}

		void ShaderPass::BindResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			if (!resource_descriptor)
			{
				auto& descriptor_allocator = execution_context.GetCurrentDescriptorAllocator();
				resource_descriptor = descriptor_allocator.allocate_descriptors(root_signature);
			}

			auto& reflection = shader->get_reflection();

			auto found = std::find_if(reflection.shader_resources.begin(), reflection.shader_resources.end(), 
				[&binding_point](const edge::gfx::ShaderResource& resource) 
				{ return resource.name_hash == binding_point; });

			if (found != reflection.shader_resources.end())
			{
				auto& render_resource = execution_context.GetResource(resource_index);

				// Bind resources
				const edge::gfx::ShaderResource& refl_resource = *found;
				if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
				{
					// Create texture descriptor
					edge::gfx::TextureDescriptor descriptor{};
					descriptor.type = refl_resource.type;
					descriptor.set = refl_resource.set;
					descriptor.binding = refl_resource.binding;

					descriptor.texture = execution_context.GetTexture(render_resource);
					descriptor.sampler = execution_context.GetSampler(render_resource);
					descriptor.subresource_index = subresource_index;
					descriptor.resource_state = render_resource.GetState();

					resource_descriptor->bind_texture(descriptor);
				}
				else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer)
				{
					// And also create resource descriptor
					edge::gfx::BufferDescriptor descriptor{};
					descriptor.type = refl_resource.type;
					descriptor.set = refl_resource.set;
					descriptor.binding = refl_resource.binding;

					descriptor.buffer = execution_context.GetBuffer(render_resource);
					descriptor.size = descriptor.buffer->get_size();

					resource_descriptor->bind_buffer(descriptor);
				}
				else
				{
					assert(false && "UNKNOWN RESOURCE TYPE");
				}
			}
		}

		void ShaderPass::BindReadResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index)
		{
			ReadResource(binding_point, resource_index, subresource_index);
			BindResource(binding_point, resource_index, subresource_index);
		}

		void ShaderPass::BindWriteResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index)
		{
			WriteResource(binding_point, resource_index, subresource_index);
			BindResource(binding_point, resource_index, subresource_index);
		}

		void ShaderPass::DispatchForTarget(edge::gfx::ICommandList& command_list, uint32_t target_it, bool divide_by_locals)
		{
			uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

			auto texture = execution_context.GetTexture(target_it);

			group_x = texture->get_width();
			group_y = texture->get_height();

			if(divide_by_locals)
				DivideWorkgroupByLocalSizes(group_x, group_y, group_z);

			command_list.dispatch(group_x, group_y, group_z);
		}

		void ShaderPass::FinalizeTarget(edge::gfx::ICommandList& command_list, uint32_t binding_point, uint32_t target)
		{
			ReadResource(binding_point, target, ~0u);
			ResolveDependencies(command_list);

			auto& resource = execution_context.GetResource(target);
			resource.finalized = true;
		}

		bool ShaderPass::CanExecute() const
		{
			return can_execute;
		}
	}
}