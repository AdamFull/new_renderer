surface_resolution = render_context:get_surface_resolution()

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_srgb)
    target_ci.type = TexType.two_dim | TexType.samplable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    
    render_context:create_render_target("zbuffer_albedo_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format()
    target_ci.type = TexType.two_dim | TexType.depth | TexType.samplable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y

    render_context:create_render_target("zbuffer_depth_target", target_ci)
end

-- Present image targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_present_format()
    target_ci.type = TexType.two_dim
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    target_ci.present_image = true

    render_context:create_render_target("present_target", target_ci)
end

-- Z-buffer pass
do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.visibility_buffer
    step_ci.rendering.queue = QueueType.graphics
    step_ci.material_pass = true

    step_ci.rendering.output_targets:add(render_context:get_texture_resource("zbuffer_albedo_target"))
    step_ci.rendering.depth_target = step_ci.rendering.stencil_target = render_context:get_texture_resource("zbuffer_depth_target")

    render_interface:create_renderer(step_ci)
end

-- Present pass
do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.fullscreen
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "testing:screenspace"
    
    -- renderer_ci.input_targets:set("zbuffer_albedo_target", render_context.get_texture_resource("zbuffer_albedo_target"))
    -- renderer_ci.input_targets:set("zbuffer_normal_target", render_context.get_texture_resource("zbuffer_normal_target"))
    -- renderer_ci.input_targets:set("zbuffer_pbr_target", render_context.get_texture_resource("zbuffer_pbr_target"))
    -- renderer_ci.input_targets:set("zbuffer_emission_target", render_context.get_texture_resource("zbuffer_emission_target"))
    -- renderer_ci.input_targets:set("zbuffer_depth_target", render_context.get_texture_resource("zbuffer_depth_target"))
    -- renderer_ci.input_targets:set("shadowmap_composition_target"render_context.get_texture_resource("shadowmap_composition_target"))
    -- step_ci.rendering.input_targets:set("penis", render_context.get_texture_resource("penis"))
    step_ci.rendering.output_targets:add(render_context:get_texture_resource("present_target"))
    
    render_interface:create_renderer(step_ci)
end