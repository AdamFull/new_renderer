

function on_create(shader_name)
	local shader_ci = GraphicsShaderCI.new()
	shader_ci.cull_mode = CullMode.front
	shader_ci.front_face = FrontFace.ccw
	shader_ci.primitive_topology = PrimitiveTopology.triangle_list

	this:get_render_targets(shader_ci)

	local shader_id = render_interface:create_shader(shader_name, shader_ci)
	local reflection = render_context:get_shader_reflection(shader_id)

	local shader = this:get_shader()
	shader:create(shader_id, reflection)
end

function on_render(delta_time)
	local surface_resolution = render_context:get_surface_resolution()

	local rendering_info = this:get_rendering_info()

	rendering_info.scissor_rect = F32Rect2D.new(0, 0, surface_resolution.x, surface_resolution.y)
	rendering_info.render_area = F32Rect2D.new(0, 0, surface_resolution.x, surface_resolution.y)

	render_context:begin_rendering(rendering_info)

	local shader = this:get_shader()
	local shader_resource_update = shader:get_resource_update_data()
	render_context:update_shader_resources(shader_resource_update, ShaderUpdateFlags.all)

	local shader_id = shader:get_id()
	render_context:bind_shader(shader_id)
	render_context:bind_shader_resources(shader_id)

	-- can bind other buffers and other things

	render_context:draw(3, 1, 0, 0)

	render_context:end_rendering(rendering_info)
end