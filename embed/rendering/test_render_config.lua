use_low_precision_velocity = true

bloom_blur_depth = 5

-- Loading test cubemap
do
    render_graph:add_static_resource("CasualDay4K", "/root/CasualDay4K.ktx2")
end

-- GI targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rg16_sfloat)
    target_ci.type:set(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_fixed_size_2d(1024)

    render_graph:add_render_resource("brdf_lut_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba32_sfloat)
    target_ci.type:set(TexType.two_dim, TexType.cubemap, TexType.samplable, TexType.storage)
    target_ci:set_fixed_size_2d(64)

    render_graph:add_render_resource("irradiance_cube_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba16_sfloat)
    target_ci.type:set(TexType.two_dim, TexType.cubemap, TexType.samplable, TexType.storage)
    target_ci:set_fixed_size_2d(512)
    target_ci:enable_mip_map()
    
   render_graph:add_render_resource("prefiltered_cube_target", target_ci)
end

-- Shadow targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(false)
    target_ci.type:set(TexType.two_dim, TexType.array, TexType.samplable)
    target_ci:set_fixed_size_2d(global_settings.cascade_shadowmap_resolution)
    target_ci.array_layers = global_settings.cascade_shadowmap_cascade_count * global_settings.max_directional_light_shadow_count

    render_graph:add_render_resource("cascaded_shadows_target", target_ci)
end

-- Visibility buffer targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type:set(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_geometry_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type:set(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_matobj_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type:set(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_skinvbo_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(false)
    target_ci.type:set(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("zbuffer_depth_target", target_ci)
end

do
    local target_ci = TextureCI.new()

    target_ci.format = render_context:get_api_format(use_low_precision_velocity and Format.rg8_unorm or Format.rg16_sfloat)
    target_ci.type:set(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("motion_vectors_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.r11g11b10_ufloat)
    target_ci.type:set(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("final_lighting_target", target_ci)
end

-- eye adaptation targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.r32_sfloat)
    target_ci.type:set(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_fixed_size_2d(1)

    render_graph:add_render_resource("avg_brightness_target", target_ci)
end

do 
    local buffer_ci = BufferCI.new();
    buffer_ci.type:set(BufferType.storage)
    buffer_ci.size = 4
    buffer_ci.count = 256

    render_graph:add_render_resource("histogram_buffer", buffer_ci)
end

-- Bloom targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.r11g11b10_ufloat)
    target_ci.type:set(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_size_scale(0.5)

    render_graph:add_render_resource("brightness_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.r11g11b10_ufloat)
    target_ci.type:set(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_size_scale(0.5)
    target_ci.mip_levels = bloom_blur_depth

    render_graph:add_render_resource("bloom_target", target_ci)
end

-- Present image targets
do
    render_graph:set_backbuffer_resource_name("present_target")
end


-- Register viewports
do
    local viewport_ci = ViewportCI.new()
    viewport_ci:depend_on_target("present_target")

    render_graph:add_viewport("default_viewport", viewport_ci)
    render_graph:set_default_viewport("default_viewport")
end

do 
    local viewport_ci = ViewportCI.new()
    viewport_ci:depend_on_viewport("default_viewport")
    viewport_ci:set_scale(0.5)

    render_graph:add_viewport("half_resolution_viewport", viewport_ci)
end

do 
    local viewport_ci = ViewportCI.new()
    -- viewport_ci:depend_on_target("cascaded_shadows_target")
    viewport_ci:set_fixed_size_2d(global_settings.cascade_shadowmap_resolution)

    render_graph:add_viewport("cascaded_shadows_viewport", viewport_ci)
end

-- Some functions
function make_compute_pass(type, queue, shader)
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.type = type
    pass_ci.queue = queue
    pass_ci.shader = shader

    return pass_ci
end

function make_graphics_pass(type, queue, shader)
    local pass_ci = GraphicsCreateInfo.new()
    pass_ci.type = type
    pass_ci.queue = queue
    pass_ci.shader = shader

    return pass_ci
end

function generate_prefiltered_cube()
    render_graph:push_debug_marker("Prefiltered cubemap generator", 0xffff33ff, QueueType.graphics)



    render_graph:pop_debug_marker("Prefiltered cubemap generator", QueueType.graphics)
end

function compute_bloom()
    render_graph:push_debug_marker("Compute bloom", 0xffff33ff, QueueType.graphics)

    -- Starting from brightness detection pass
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:brightdetect")

        pass_ci:add_workgroup_size_info("bloom_target", true)

        pass_ci:bind_read_resource("sampler_color", "final_lighting_target")
        pass_ci:bind_write_resource("write_color", "brightness_target")

        render_graph:add_render_pass("brightdetect_pass", pass_ci)
    end

    -- Begin downsampling
    render_graph:push_debug_marker("Downsample", 0xffff33ff, QueueType.graphics)
    for idx = 0, bloom_blur_depth - 1 do
        local pass_ci = ComputeCreateInfo.new()
        pass_ci.type = ComputeType.default
        pass_ci.queue = QueueType.graphics
        pass_ci.shader = (idx == 0) and "postprocess:downsample:FIRST_PASS" or "postprocess:downsample"

        current_target = (idx == 0) and "brightness_target" or "bloom_target"
        current_target_idx = (idx == 0) and 0 or (idx - 1)

        pass_ci:add_workgroup_size_info("bloom_target", idx, true)
        
        pass_ci:bind_read_resource("sampler_color", current_target, current_target_idx)
        pass_ci:bind_write_resource("write_color", "bloom_target", idx)
        
        render_graph:add_render_pass("downsample_pass_" .. idx, pass_ci)
    end
    render_graph:pop_debug_marker("Downsample", QueueType.graphics)

    -- And upsampling
    render_graph:push_debug_marker("Upsample", 0xffff33ff, QueueType.graphics)
    for idx = bloom_blur_depth - 1, 1, -1 do
        local pass_ci = ComputeCreateInfo.new()
        pass_ci.type = ComputeType.default
        pass_ci.queue = QueueType.graphics
        pass_ci.shader = "postprocess:upsample"
    
        pass_ci:add_workgroup_size_info("bloom_target", idx - 1, true)
    
        pass_ci:bind_read_resource("sampler_color", "bloom_target", idx)
        pass_ci:bind_write_resource("write_color", "bloom_target", idx - 1)
    
        render_graph:add_render_pass("upnsample_pass_" .. idx, pass_ci)
    end
    render_graph:pop_debug_marker("Upsample", QueueType.graphics)

    render_graph:pop_debug_marker("Compute bloom", QueueType.graphics)
end


-- generate brdf lut
do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.one_time = true
    pass_ci.type = ComputeType.default
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "lighting:brgflut_generator";

    pass_ci:add_workgroup_size_info("brdf_lut_target", true)
    pass_ci:bind_write_resource("out_color", "brdf_lut_target")

    render_graph:add_render_pass("generate_brdflut", pass_ci)
end

do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.one_time = true
    pass_ci.type = ComputeType.default
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "lighting:irradiancecube_generator";

    pass_ci:add_workgroup_size_info("irradiance_cube_target", true)
    pass_ci:bind_write_resource("out_color", "irradiance_cube_target")
    pass_ci:bind_read_resource("sampler_color", "CasualDay4K")

    render_graph:add_render_pass("generate_irradiance_cube", pass_ci)
end

do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.one_time = true
    pass_ci.type = ComputeType.cubemap
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "lighting:prefilteredmap_generator";

    pass_ci:add_workgroup_size_info("prefiltered_cube_target", true)
    pass_ci:bind_write_resource("out_color", "prefiltered_cube_target")
    pass_ci:bind_read_resource("sampler_color", "CasualDay4K")

    render_graph:add_render_pass("generate_prefiltered_cube", pass_ci)
end


do
    local pass_ci = GraphicsCreateInfo.new()
    pass_ci.type = RenderingType.csm
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "lighting:unidirectional_shadow:CASCADED_SHADOW"
    
    pass_ci:set_viewport("cascaded_shadows_viewport")
    pass_ci:set_depth_target("cascaded_shadows_target")

    pass_ci:bind_read_resource("nodeBuffer", "NodeStorage")
    pass_ci:bind_read_resource("materialBuffer", "MaterialStorage")
    pass_ci:bind_read_resource("jointsBuffer", "SkinningJoints")
    pass_ci:bind_read_resource("cascadedShadowBuffer", "CascadedShadows")
    pass_ci:bind_read_resource("batchBuffer", "GeometryBatches")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")

    render_graph:add_render_pass("cascaded_shadow_map_pass", pass_ci)
end

-- visibility buffer write
do
    local pass_ci = GraphicsCreateInfo.new()
    pass_ci.type = RenderingType.geometry
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "visibilitybuffer:write"

    pass_ci:set_depth_target("zbuffer_depth_target")
    pass_ci:set_stencil_target("zbuffer_depth_target")

    pass_ci:bind_read_resource("nodeBuffer", "NodeStorage")
    pass_ci:bind_read_resource("jointsBuffer", "SkinningJoints")

    pass_ci:add_write_resource("visbuffer_geometry_target")
    pass_ci:add_write_resource("visbuffer_matobj_target")
    pass_ci:add_write_resource("visbuffer_skinvbo_target")
    --pass_ci:add_write_resource("motion_vectors_target")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")

    render_graph:add_render_pass("visibility_buffer_write_pass", pass_ci)
end

-- visibility buffer material pass
do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.type = ComputeType.geometry
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "visibilitybuffer:shade"

    pass_ci:add_workgroup_size_info("final_lighting_target", true)

    pass_ci:bind_write_resource("write_color", "final_lighting_target")
    pass_ci:bind_read_resource("visbuffer_geometry", "visbuffer_geometry_target")
    pass_ci:bind_read_resource("visbuffer_matobj", "visbuffer_matobj_target")
    pass_ci:bind_read_resource("visbuffer_skinvbo", "visbuffer_skinvbo_target")
    pass_ci:bind_read_resource("brdf_lut", "brdf_lut_target")
    pass_ci:bind_read_resource("irradiance_cube", "irradiance_cube_target")
    pass_ci:bind_read_resource("prefiltered_cube", "prefiltered_cube_target")
    pass_ci:bind_read_resource("cascaded_shadowmap", "cascaded_shadows_target")
    pass_ci:bind_read_resource("depth_buffer", "zbuffer_depth_target")

    pass_ci:bind_read_resource("nodeBuffer", "NodeStorage")
    pass_ci:bind_read_resource("materialBuffer", "MaterialStorage")
    pass_ci:bind_read_resource("jointsBuffer", "SkinningJoints")
    pass_ci:bind_read_resource("directionalLightBuffer", "DirectionalLights")
    pass_ci:bind_read_resource("pointLightBuffer", "PointLights")
    pass_ci:bind_read_resource("spotLightBuffer", "SpotLights")
    pass_ci:bind_read_resource("cascadedShadowBuffer", "CascadedShadows")
    pass_ci:bind_read_resource("omniShadowBuffer", "OmniShadows")
    pass_ci:bind_read_resource("spotShadowBuffer", "SpotShadows")
    pass_ci:bind_read_resource("batchBuffer", "GeometryBatches")

    pass_ci:bind_read_resource("indirectCommandBuffer", "IndexedDrawIndirectBuffer")

    render_graph:add_render_pass("compute_materials_pass", pass_ci)
end

-- Compute histogram
do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.type = ComputeType.default
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "postprocess:compute_histogram"

    pass_ci:add_workgroup_size_info("final_lighting_target", true)

    pass_ci:bind_read_resource("sampler_color", "final_lighting_target")
    pass_ci:bind_write_resource("Histogram", "histogram_buffer")

    render_graph:add_render_pass("compute_histogram_pass", pass_ci)
end

-- Compute avg screen space brightness
do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.type = ComputeType.default
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "postprocess:compute_brightness"

    pass_ci:add_workgroup_size_info(1, 1, 1, false)

    pass_ci:bind_write_resource("target", "avg_brightness_target")
    pass_ci:bind_read_resource("Histogram", "histogram_buffer")

    render_graph:add_render_pass("compute_brightness_pass", pass_ci)
end

-- Compute static motion vectors (can be started in another queue)
do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.type = ComputeType.default
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = use_low_precision_velocity and "postprocess:generate_motion_vectors:LOW_PRECISION_VELOCITY" or "postprocess:generate_motion_vectors"

    pass_ci:add_workgroup_size_info("zbuffer_depth_target", true)

    pass_ci:bind_write_resource("write_color", "motion_vectors_target")
    pass_ci:bind_write_resource("depth_texture", "zbuffer_depth_target")

    render_graph:add_render_pass("generate_motion_vectors_pass", pass_ci)
    
end

-- Compute bloom
--compute_bloom()

-- Present pass
do
    local pass_ci = GraphicsCreateInfo.new()
    pass_ci.type = RenderingType.fullscreen
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = use_low_precision_velocity and "testing:screenspace:LOW_PRECISION_VELOCITY" or "testing:screenspace"
    pass_ci:flip_viewport()

    pass_ci:bind_read_resource("input_image", "final_lighting_target")
    pass_ci:bind_read_resource("avg_luminosity", "avg_brightness_target")
    pass_ci:bind_read_resource("motion_vectors", "motion_vectors_target")
    pass_ci:bind_read_resource("bloom_blur", "bloom_target")
    pass_ci:add_write_resource("present_target")

    render_graph:add_render_pass("finalize_image_pass", pass_ci)
end