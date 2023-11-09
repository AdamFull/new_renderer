surface_resolution = render_context:get_surface_resolution()

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

-- Present pass
do
    local renderer_ci = RendererCI.new()
    renderer_ci.render_script = "/root/embed/rendering/render_stages/fullscreen.lua"
    renderer_ci.queue = QueueType.present
    renderer_ci.shader = render_interface:get_shader_desc("fullscreen:test")
    
    -- renderer_ci.input_targets:set("zbuffer_albedo_target", render_context.get_resource("zbuffer_albedo_target", ResType.rt))
    -- renderer_ci.input_targets:set("zbuffer_normal_target", render_context.get_resource("zbuffer_normal_target", ResType.rt))
    -- renderer_ci.input_targets:set("zbuffer_pbr_target", render_context.get_resource("zbuffer_pbr_target", ResType.rt))
    -- renderer_ci.input_targets:set("zbuffer_emission_target", render_context.get_resource("zbuffer_emission_target", ResType.rt))
    -- renderer_ci.input_targets:set("zbuffer_depth_target", render_context.get_resource("zbuffer_depth_target", ResType.rt))
    -- renderer_ci.input_targets:set("shadowmap_composition_target"render_context.get_resource("shadowmap_composition_target", ResType.rt))
    renderer_ci.output_targets:add(render_context:get_resource("present_target", ResType.rt))
    
    render_interface:create_renderer(renderer_ci)
end