

function on_create(shader_name)
	local shader_ci = GraphicsShaderCI.new()
	shader_ci.cull_mode = CullMode.front
	shader_ci.front_face = FrontFace.ccw
	shader_ci.primitive_topology = PrimitiveTopology.triangle_list

	this:get_render_targets(shader_ci)

	local shader_view = render_interface:create_shader(shader_name, shader_ci)
	local reflection = render_context:get_shader_reflection(shader_view)

	local shader = this:get_shader()
	shader:create(shader_view, reflection)
end

function on_render(delta_time)
end