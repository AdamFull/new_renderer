#include "ShaderPass.h"

#include "../RendererInterface.h"
#include <edge/gfx/gfx_context.h>
#include <edge/gfx/gfx_util.h>

#include <edge/util/Logger.h>
#include <edge/util/utl_hash.hpp>

#include <edge/util/utl_profiler.h>
#include <edge/gfx/gfx_profiler.h>

namespace engine::render {
	ShaderPass::ShaderPass(IExecutionContext& execution_context, const PassCreateInfo& create_info) :
		debug_name{ create_info.name },
		shader{ create_info.shader },
		root_signature{ create_info.root_signature },
		pipeline{ create_info.pipeline },
		descriptor_layouts{ create_info.descriptor_layouts },
		execution_context{ execution_context },
		one_time{ create_info.one_time },
		draw_callback{ create_info.draw_callback },
		enabled_callback{ create_info.enabled_callback } {
		auto& reflection = shader->get_reflection();

		// Create uniform block handlers for resources
		for (auto it = reflection.shader_resources.begin(); it != reflection.shader_resources.end(); ++it) {
			// TODO: this is bad, because in most cases set 0 is global, but in some other is not
			if (it->type == edge::gfx::DescriptorType::eUniformBuffer && it->set != 0) {
				uniform_handles.emplace_back(reflection, (*it));
			}
		}

		// Also add push constant block
		constant_block = std::make_unique<UniformHandle>(reflection, reflection.constants);
	}

	ShaderPass::~ShaderPass() {
	}


	void ShaderPass::execute(edge::gfx::ICommandList& command_list, float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		if (enabled_callback && !enabled_callback()) {
			return;
		}

		EDGE_GFX_PROFILER_BEGIN_ZONE_NC(command_list, debug_name, 0xFFFF11FF);
		if (can_execute_) {
			command_list.bind_pipeline(pipeline);
			_execute(command_list, delta_time);
		}
		EDGE_GFX_PROFILER_END_ZONE(command_list);

		// Update one time flag for single time passes
		if (one_time) {
			can_execute_ = false;
		}
	}

	UniformHandle& ShaderPass::get_uniform_block(uint32_t name_hash) {
		auto found = std::find_if(uniform_handles.begin(), uniform_handles.end(), 
			[&name_hash](const UniformHandle& handle) { return handle.get_name_hash() == name_hash; });

		if (found != uniform_handles.end()) {
			return *found;
		}

		assert(false && "Requested uniform block is not exists");

		return *constant_block;
	}

	UniformHandle& ShaderPass::get_uniform_block(const char* name) {
		return get_uniform_block(edge::murmur_hash(name));
	}

	void ShaderPass::bind_resources(edge::gfx::ICommandList& command_list) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto binding_layouts = execution_context.get_binding_layouts(descriptor_layouts);
		if (binding_layouts.empty()) {
			// ERROR
			return;
		}

		// Update uniform buffers
		for (auto& handle : uniform_handles) {
			const auto& resource = handle.get_resource();

			// Request memory block for this uniform
			// TODO: may be request it inside uniform handler?
			auto uniform_block = execution_context.request_uniform_block(resource.size);
			uniform_block.update(std::span(handle.data()));

			edge::gfx::BufferDescriptor descriptor{};
			descriptor.type = resource.type;
			descriptor.binding = resource.binding;

			descriptor.buffer = &uniform_block.get_buffer();
			descriptor.size = static_cast<uint32_t>(uniform_block.get_size());
			descriptor.offset = static_cast<uint32_t>(uniform_block.get_offset());

			binding_layouts[resource.set]->bind(descriptor);
		}

		edge::gfx::BindDescriptorsInfo bind_info{};
		bind_info.pipeline_type = pipeline->get_type();
		bind_info.binding_layouts = binding_layouts;
		bind_info.root_signature = root_signature;

		command_list.bind_descriptors(bind_info);
	}

	void ShaderPass::push_constant_ranges(edge::gfx::ICommandList& command_list) {
		auto& constant_data = constant_block->data();
		command_list.push_constants(shader, root_signature, std::span<const char>(constant_data.data(), constant_data.size()));
	}

	void ShaderPass::divide_workgroup_by_local_sizes(uint32_t& group_x, uint32_t& group_y, uint32_t& group_z) {
		auto& reflection = shader->get_reflection();
		group_x = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_x) / static_cast<float>(reflection.local_sizes[0]))), 1u);
		group_y = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_y) / static_cast<float>(reflection.local_sizes[1]))), 1u);
		group_z = std::max(static_cast<uint32_t>(std::ceil(static_cast<float>(group_z) / static_cast<float>(reflection.local_sizes[2]))), 1u);
	}

	void ShaderPass::resource_dependency(uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& render_resource = execution_context.get_resource(resource_id);

		edge::gfx::ResourceStateFlags new_state{};

		auto pass_type = get_pass_type();
		if (pass_type == ShaderPassType::eCompute) {
			new_state |= edge::gfx::ResourceStateFlag::eNonGraphicsShader;
		}
		else if (pass_type == ShaderPassType::eGraphics) {
			new_state |= edge::gfx::ResourceStateFlag::eGraphicsShader;
		}

		if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage) {
			// May be create another tag like unordered resource?
			if (render_resource.resource_type & RenderResourceTypeFlag::eStorageTarget) {
				new_state |= edge::gfx::ResourceStateFlag::eUnorderedAccess;
			}
			else {
				new_state |= edge::gfx::ResourceStateFlag::eShaderResource;
			}

			auto& barrier = texture_barriers.emplace_back();
			barrier.texture = execution_context.get_texture(render_resource);
			barrier.subresource_index = sebresource_id;
			barrier.src_state = render_resource.GetState();
			barrier.dst_state = new_state;
		}
		else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer) {
			if (render_resource.resource_type & RenderResourceTypeFlag::eStorageBuffer) {
				new_state |= edge::gfx::ResourceStateFlag::eUnorderedAccess;
			}

			if (render_resource.resource_type & RenderResourceTypeFlag::eIndirectBuffer) {
				new_state |= edge::gfx::ResourceStateFlag::eIndirectArgument;
			}

			auto& barrier = buffer_barriers.emplace_back();
			barrier.buffer = execution_context.get_buffer(render_resource);
			barrier.src_state = render_resource.GetState();
			barrier.dst_state = new_state;
		}

		render_resource.SetState(new_state);
	}

	void ShaderPass::resource_dependency(uint32_t binding_point, uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto& reflection = shader->get_reflection();
		auto found = std::find_if(reflection.shader_resources.begin(), reflection.shader_resources.end(),
			[&binding_point](const edge::gfx::ShaderResource& resource)
			{ return resource.name_hash == binding_point; });

		if (found != reflection.shader_resources.end()) {
			auto& render_resource = execution_context.get_resource(resource_id);

			edge::gfx::ResourceStateFlags new_state{};

			auto pass_type = get_pass_type();
			if (pass_type == ShaderPassType::eCompute) {
				new_state |= edge::gfx::ResourceStateFlag::eNonGraphicsShader;
			}
			else if (pass_type == ShaderPassType::eGraphics) {
				new_state |= edge::gfx::ResourceStateFlag::eGraphicsShader;
			}

			const edge::gfx::ShaderResource& refl_resource = *found;
			new_state |= edge::gfx::get_state_by_descriptor_type(refl_resource.type);

			if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage) {
				// May be create another tag like unordered resource?
				if (render_resource.resource_type & RenderResourceTypeFlag::eStorageTarget) {
					new_state |= edge::gfx::ResourceStateFlag::eUnorderedAccess;
				}
				else {
					new_state |= edge::gfx::ResourceStateFlag::eShaderResource;
				}

				if (render_resource.current_state == new_state) {
					return;
				}

				auto& barrier = texture_barriers.emplace_back();
				barrier.texture = execution_context.get_texture(render_resource);
				barrier.subresource_index = sebresource_id;
				barrier.src_state = render_resource.GetState();
				barrier.dst_state = new_state;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer) {
				if (render_resource.current_state == new_state) {
					return;
				}

				auto& barrier = buffer_barriers.emplace_back();
				barrier.buffer = execution_context.get_buffer(render_resource);
				barrier.src_state = render_resource.GetState();
				barrier.dst_state = new_state;
			}

			render_resource.SetState(new_state);
		}
	}

	UniformHandle& ShaderPass::get_constant_block() {
		return *constant_block;
	}

	void ShaderPass::read_resource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index) {
		resource_dependency(binding_point, render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eRead);
	}

	void ShaderPass::read_resource(uint32_t render_resource_id, uint32_t subresource_index) {
		resource_dependency(render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eRead);
	}

	void ShaderPass::write_resource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index) {
		resource_dependency(binding_point, render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eWrite);
	}

	void ShaderPass::write_resource(uint32_t render_resource_id, uint32_t subresource_index) {
		resource_dependency(render_resource_id, subresource_index, edge::gfx::AccessTypeFlag::eWrite);
	}

	void ShaderPass::resolve_dependencies(edge::gfx::ICommandList& command_list) {
		// Flush resource barriers
		// Prepare barriers
		resource_barriers.buffer_barriers = buffer_barriers;
		resource_barriers.texture_barriers = texture_barriers;
		resource_barriers.memory_barriers = memory_barriers;

		command_list.resource_barrier(resource_barriers);

		buffer_barriers.clear();
		texture_barriers.clear();
		memory_barriers.clear();
	}

	void ShaderPass::bind_resource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index) {
		EDGE_PROFILE_ZONE_SCOPED;

		auto binding_layouts = execution_context.get_binding_layouts(descriptor_layouts);
		if (binding_layouts.empty()) {
			// ERROR
			return;
		}

		auto& reflection = shader->get_reflection();

		auto found = std::find_if(reflection.shader_resources.begin(), reflection.shader_resources.end(),
			[&binding_point](const edge::gfx::ShaderResource& resource)
			{ return resource.name_hash == binding_point; });

		if (found != reflection.shader_resources.end()) {
			auto& render_resource = execution_context.get_resource(resource_index);

			// Bind resources
			const edge::gfx::ShaderResource& refl_resource = *found;
			if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage) {
				// Create texture descriptor
				edge::gfx::TextureDescriptor descriptor{};
				descriptor.type = refl_resource.type;
				descriptor.binding = refl_resource.binding;

				descriptor.texture = execution_context.get_texture(render_resource);
				descriptor.subresource_index = subresource_index;
				descriptor.resource_state = render_resource.GetState();

				binding_layouts[refl_resource.set]->bind(descriptor);
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllBuffer) {
				// And also create resource descriptor
				edge::gfx::BufferDescriptor descriptor{};
				descriptor.type = refl_resource.type;
				descriptor.binding = refl_resource.binding;

				descriptor.buffer = execution_context.get_buffer(render_resource);
				descriptor.size = static_cast<uint32_t>(descriptor.buffer->get_size());

				binding_layouts[refl_resource.set]->bind(descriptor);
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eSampler) {
				edge::gfx::SamplerDescriptor descriptor{};
				descriptor.binding = refl_resource.binding;
				descriptor.sampler = execution_context.get_sampler(render_resource);

				binding_layouts[refl_resource.set]->bind(descriptor);
			}
			else {
				assert(false && "UNKNOWN RESOURCE TYPE");
			}
		}
	}

	void ShaderPass::bind_read_resource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index) {
		read_resource(binding_point, resource_index, subresource_index);
		bind_resource(binding_point, resource_index, subresource_index);
	}

	void ShaderPass::bind_write_resource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index) {
		write_resource(binding_point, resource_index, subresource_index);
		bind_resource(binding_point, resource_index, subresource_index);
	}

	void ShaderPass::dispatch_for_buffer(edge::gfx::ICommandList& command_list, uint32_t buffer_id, bool divide_by_locals) {
		uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

		auto buffer = execution_context.get_buffer(buffer_id);

		group_x = static_cast<uint32_t>(buffer->get_size());

		if (divide_by_locals) {
			divide_workgroup_by_local_sizes(group_x, group_y, group_z);
		}

		command_list.dispatch(group_x, group_y, group_z);
	}

	void ShaderPass::dispatch_for_target(edge::gfx::ICommandList& command_list, uint32_t target_it, bool divide_by_locals) {
		uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

		auto texture = execution_context.get_texture(target_it);

		group_x = texture->get_width();
		group_y = texture->get_height();
		group_z = texture->get_depth();

		if (divide_by_locals)
			divide_workgroup_by_local_sizes(group_x, group_y, group_z);

		command_list.dispatch(group_x, group_y, group_z);
	}

	bool ShaderPass::can_execute() const {
		return can_execute_;
	}
}