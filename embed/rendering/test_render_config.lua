global_settings = 
{
    max_directional_light_count = 3,
    max_directional_light_shadow_count = 1,
    cascade_shadowmap_resolution = 2048,
    cascade_shadowmap_cascade_count = 8,

    max_spot_light_count = 64,
    max_spot_light_shadow_count = 16,
    spotlight_shadowmap_resolution = 1024,

    max_point_light_count = 64,
    max_point_light_shadow_count = 16,
    point_light_shadowmap_resolution = 512
}

surface_resolution = render_context:get_surface_resolution()

-- Loading test cubemap
do
    render_interface:create_texture("/root/CasualDay4K.ktx2")
end

-- GI targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rg16_sfloat)
    target_ci.type = bit_or(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci.size_policy = SizePolicy.fixed
    target_ci.width = 1024
    target_ci.height = 1024

    render_context:create_render_target("brdf_lut_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba32_sfloat)
    target_ci.type = bit_or(TexType.two_dim, TexType.cubemap, TexType.samplable, TexType.storage)
    target_ci.size_policy = SizePolicy.fixed
    target_ci.width = 64
    target_ci.height = 64

    render_context:create_render_target("irradiance_cube_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba16_sfloat)
    target_ci.type = bit_or(TexType.two_dim, TexType.cubemap, TexType.samplable, TexType.storage)
    target_ci.size_policy = SizePolicy.fixed
    target_ci.width = 512
    target_ci.height = 512
    target_ci.mip_levels = calculate_mip_level_count(512, 512)
    
    render_context:create_render_target("prefiltered_cube_target", target_ci)
end

-- Shadow targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(false)
    target_ci.type = bit_or(TexType.two_dim, TexType.array, TexType.samplable)
    target_ci.size_policy = SizePolicy.fixed
    target_ci.width = global_settings.cascade_shadowmap_resolution
    target_ci.height = global_settings.cascade_shadowmap_resolution
    target_ci.array_layers = global_settings.cascade_shadowmap_cascade_count * global_settings.max_directional_light_shadow_count

    render_context:create_render_target("cascaded_shadows_target", target_ci)
end

-- Visibility buffer targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type = bit_or(TexType.two_dim, TexType.samplable)
    target_ci.size_policy = SizePolicy.scalable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    
    render_context:create_render_target("visbuffer_geometry_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type = bit_or(TexType.two_dim, TexType.samplable)
    target_ci.size_policy = SizePolicy.scalable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    
    render_context:create_render_target("visbuffer_matobj_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type = bit_or(TexType.two_dim, TexType.samplable)
    target_ci.size_policy = SizePolicy.scalable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    
    render_context:create_render_target("visbuffer_skinvbo_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(false)
    target_ci.type = bit_or(TexType.two_dim, TexType.samplable)
    target_ci.size_policy = SizePolicy.scalable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y

    render_context:create_render_target("zbuffer_depth_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba16_sfloat)
    target_ci.type = bit_or(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci.size_policy = SizePolicy.scalable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y

    render_context:create_render_target("final_lighting_target", target_ci)
end

-- Present image targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_present_format()
    target_ci.type = TexType.two_dim
    target_ci.size_policy = SizePolicy.full
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    target_ci.present_image = true

    render_context:create_render_target("present_target", target_ci)
end








-- generate brdf lut
do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.compute
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "lighting:brgflut_generator"
    step_ci.rendering.one_time = true

    target_img = render_context:get_texture_resource("brdf_lut_target")
    step_ci.rendering.output_targets:add(target_img)
    step_ci.rendering.input_targets:set("out_color", target_img)
    step_ci.rendering.render_rect = render_context:get_render_rect_from_rt(target_img)

    render_interface:create_renderer(step_ci)
end

do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.compute
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "lighting:irradiancecube_generator"
    step_ci.rendering.one_time = true

    target_img = render_context:get_texture_resource("irradiance_cube_target")
    step_ci.rendering.output_targets:add(target_img)
    step_ci.rendering.input_targets:set("out_color", target_img)
    step_ci.rendering.input_targets:set("sampler_color", render_context:get_texture_resource("CasualDay4K"))
    step_ci.rendering.render_rect = render_context:get_render_rect_from_rt(target_img)

    render_interface:create_renderer(step_ci)
end

do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.compute_cubemap
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "lighting:prefilteredmap_generator"
    step_ci.rendering.one_time = true

    target_img = render_context:get_texture_resource("prefiltered_cube_target")
    step_ci.rendering.output_targets:add(target_img)
    step_ci.rendering.input_targets:set("out_color", target_img)
    step_ci.rendering.input_targets:set("sampler_color", render_context:get_texture_resource("CasualDay4K"))
    step_ci.rendering.render_rect = render_context:get_render_rect_from_rt(target_img)

    render_interface:create_renderer(step_ci)
end

do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.csm
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "lighting:unidirectional_shadow:CASCADED_SHADOW"

    step_ci.rendering.depth_target = render_context:get_texture_resource("cascaded_shadows_target")
    step_ci.rendering.render_rect = render_context:get_render_rect_from_rt(step_ci.rendering.depth_target)

    render_interface:create_renderer(step_ci)
end

-- visibility buffer write
do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.geometry
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "visibilitybuffer:write"
    step_ci.rendering.flip_viewport = true

    step_ci.rendering.output_targets:add(render_context:get_texture_resource("visbuffer_geometry_target"))
    step_ci.rendering.output_targets:add(render_context:get_texture_resource("visbuffer_matobj_target"))
    step_ci.rendering.output_targets:add(render_context:get_texture_resource("visbuffer_skinvbo_target"))
    step_ci.rendering.depth_target = render_context:get_texture_resource("zbuffer_depth_target")
    step_ci.rendering.stencil_target = step_ci.rendering.depth_target
    step_ci.rendering.render_rect = render_context:get_render_rect_from_rt(render_context:get_texture_resource("visbuffer_geometry_target"))

    render_interface:create_renderer(step_ci)
end

-- visibility buffer material pass
do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.compute_geometry
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "visibilitybuffer:shade"

    target_img = render_context:get_texture_resource("final_lighting_target")
    step_ci.rendering.output_targets:add(target_img)
    step_ci.rendering.input_targets:set("write_color", target_img)
    step_ci.rendering.render_rect = render_context:get_render_rect_from_rt(target_img)

    step_ci.rendering.input_targets:set("visbuffer_geometry", render_context:get_texture_resource("visbuffer_geometry_target"))
    step_ci.rendering.input_targets:set("visbuffer_matobj", render_context:get_texture_resource("visbuffer_matobj_target"))
    step_ci.rendering.input_targets:set("visbuffer_skinvbo", render_context:get_texture_resource("visbuffer_skinvbo_target"))
    step_ci.rendering.input_targets:set("brdf_lut", render_context:get_texture_resource("brdf_lut_target"))
    step_ci.rendering.input_targets:set("irradiance_cube", render_context:get_texture_resource("irradiance_cube_target"))
    step_ci.rendering.input_targets:set("prefiltered_cube", render_context:get_texture_resource("prefiltered_cube_target"))
    step_ci.rendering.input_targets:set("cascaded_shadowmap", render_context:get_texture_resource("cascaded_shadows_target"))
    step_ci.rendering.input_targets:set("depth_buffer", render_context:get_texture_resource("zbuffer_depth_target"))

    render_interface:create_renderer(step_ci)
end

-- Present pass
do
    local step_ci = RenderingStepCI.new()
    step_ci.type = RenderingStepType.rendering

    step_ci.rendering.type = RenderingType.fullscreen
    step_ci.rendering.queue = QueueType.graphics
    step_ci.rendering.shader_name = "testing:screenspace"
    
    step_ci.rendering.input_targets:set("input_image", render_context:get_texture_resource("final_lighting_target"))
    step_ci.rendering.output_targets:add(render_context:get_texture_resource("present_target"))
    step_ci.rendering.render_rect = render_context:get_render_rect_from_rt(render_context:get_texture_resource("present_target"))
    
    render_interface:create_renderer(step_ci)
end