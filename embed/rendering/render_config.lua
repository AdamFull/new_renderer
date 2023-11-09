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

-- Initializing render targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rg16_sfloat)
    target_ci.type = TexType.two_dim | TexType.samplable | TexType.storage
    target_ci.width = target_ci.height = 1024

    render_context:create_render_target("brdf_lut_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba32_sfloat)
    target_ci.type = TexType.cubemap | TexType.samplable | TexType.storage
    target_ci.width = target_ci.height = 64

    render_context:create_render_target("irradiance_cube_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba16_sfloat)
    target_ci.type = TexType.cubemap | TexType.samplable | TexType.storage
    target_ci.width = target_ci.height = 512
    
    render_context:create_render_target("prefiltered_cube_target", target_ci)
end

-- Shadowmap render targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(),
    target_ci.type = TexType.two_dim | TexType.array | TexType.depth | TexType.samplable
    target_ci.width = target_ci.height = global_settings.cascade_shadowmap_resolution
    target_ci.array_layers = global_settings.cascade_shadowmap_cascade_count * global_settings.max_directional_light_shadow_count
    
    render_context:create_render_target("cascaded_sm_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format()
    target_ci.type = TexType.two_dim | TexType.array | TexType.depth | TexType.samplable
    target_ci.width = target_ci.height = global_settings.spotlight_shadowmap_resolution
    target_ci.array_layers = global_settings.max_spot_light_shadow_count

    render_context:create_render_target("directional_sm_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format()
    target_ci.type = TexType.cubemap | TexType.array | TexType.depth | TexType.samplable
    target_ci.width = target_ci.height = global_settings.point_light_shadowmap_resolution
    target_ci.array_layers = global_settings.max_point_light_shadow_count

    render_context:create_render_target("omni_sm_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format()
    target_ci.type = TexType.two_dim | TexType.depth | TexType.samplable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y

    render_context:create_render_target("shadowmap_composition_target", target_ci)
end

-- Z-Buffer targets
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
    target_ci.format = render_context:get_api_format(Format.rgba16_sfloat)
    target_ci.type = TexType.two_dim | TexType.samplable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    
    render_context:create_render_target("zbuffer_normal_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type = TexType.two_dim | TexType.samplable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y
    
    render_context:create_render_target("zbuffer_pbr_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_srgb)
    target_ci.type = TexType.two_dim | TexType.samplable
    target_ci.width = surface_resolution.x
    target_ci.height = surface_resolution.y

    render_context:create_render_target("zbuffer_emission_target", target_ci)
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

-- Create render pipeline
