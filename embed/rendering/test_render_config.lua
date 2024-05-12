use_low_precision_velocity = true

motion_blur_enabled = true
depth_of_field_enabled = false
chromatic_aberration_enabled = false
bloom_enabled = false

bloom_blur_depth = 5

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

function swap(a, b)
    return b, a
end

function make_target(format, flags)
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(format)
    target_ci.type = ETextureCreateFlags.new(flags)
    return target_ci
end

function make_depth_target(depth_only, flags)
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(depth_only)
    target_ci.type = ETextureCreateFlags.new(flags)
    return target_ci
end

-- Loading test cubemap
do
    render_graph:add_static_resource("CasualDay4K", "/root/CasualDay4K.ktx2")
end

-- GI targets
do
    local target_ci = make_target(Format.rg16_sfloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_fixed_size_2d(1024)

    render_graph:add_render_resource("brdf_lut_target", target_ci)
end

do
    local target_ci = make_target(Format.rgba32_sfloat, TexType.two_dim | TexType.cubemap | TexType.samplable | TexType.storage)
    target_ci:set_fixed_size_2d(64)

    render_graph:add_render_resource("irradiance_cube_target", target_ci)
end

do
    local target_ci = make_target(Format.rgba16_sfloat, TexType.two_dim | TexType.cubemap | TexType.samplable | TexType.storage)
    target_ci:set_fixed_size_2d(512)
    target_ci:enable_mip_map()
    
   render_graph:add_render_resource("prefiltered_cube_target", target_ci)
end

-- Shadow targets
do
    local target_ci = make_target(Format.d32_sfloat, TexType.two_dim | TexType.array | TexType.samplable)
    target_ci:set_fixed_size_2d(global_settings.cascade_shadowmap_resolution)
    target_ci.array_layers = global_settings.cascade_shadowmap_cascade_count * global_settings.max_directional_light_shadow_count

    render_graph:add_render_resource("cascaded_shadows_target", target_ci)
end

do
    local target_ci = make_target(Format.d16_unorm, TexType.two_dim | TexType.array | TexType.samplable)
    target_ci:set_fixed_size_2d(global_settings.spotlight_shadowmap_resolution)
    target_ci.array_layers = global_settings.max_spot_light_shadow_count

    render_graph:add_render_resource("directional_shadows_target", target_ci) 
end

do
    local target_ci = make_target(Format.d16_unorm, TexType.two_dim | TexType.cubemap | TexType.array | TexType.samplable)
    target_ci:set_fixed_size_2d(global_settings.point_light_shadowmap_resolution)
    target_ci.array_layers = global_settings.max_point_light_shadow_count

    render_graph:add_render_resource("omni_shadows_target", target_ci)
end

-- Visibility buffer targets
do
    local target_ci = make_target(Format.rgba8_unorm, TexType.two_dim | TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_geometry_target", target_ci)
end

do
    local target_ci = make_target(Format.rgba8_unorm, TexType.two_dim | TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_vboinst_target", target_ci)
end

do
    local target_ci = make_depth_target(false, TexType.two_dim | TexType.samplable)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("zbuffer_depth_target", target_ci)
end

do
    local target_ci = make_target(use_low_precision_velocity and Format.rg8_unorm or Format.rg16_sfloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("motion_vectors_target", target_ci)
end

do
    local target_ci = make_target(Format.r11g11b10_ufloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("hdr0", target_ci)
end

do
    local target_ci = make_target(Format.r11g11b10_ufloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("hdr1", target_ci)
end

do
    local target_ci = make_target(Format.r11g11b10_ufloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("hdr0_history", target_ci)
end

-- eye adaptation targets
do
    local target_ci = make_target(Format.r32_sfloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_fixed_size_2d(1)

    render_graph:add_render_resource("avg_brightness_target", target_ci)
end

do 
    local buffer_ci = BufferCI.new();
    buffer_ci.type = EBufferUsageFlags.new(BufferType.storage)
    buffer_ci.size = 4
    buffer_ci.count = 256

    render_graph:add_render_resource("histogram_buffer", buffer_ci)
end

do
    local target_ci = make_target(Format.r11g11b10_ufloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("bokeh_target", target_ci)
end

-- Bloom targets
do
    local target_ci = make_target(Format.r11g11b10_ufloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_size_scale(0.5)

    render_graph:add_render_resource("brightness_target", target_ci)
end

do
    local target_ci = make_target(Format.r11g11b10_ufloat, TexType.two_dim | TexType.samplable | TexType.storage)
    target_ci:set_size_scale(0.5)
    target_ci.mip_levels = bloom_blur_depth

    render_graph:add_render_resource("bloom_target", target_ci)
end

-- Present image targets
do
    render_graph:set_backbuffer_resource_name("present_target")
end


function generate_prefiltered_cube()
    render_graph:push_debug_marker("Prefiltered cubemap generator", 0xffff33ff, QueueType.graphics)



    render_graph:pop_debug_marker("Prefiltered cubemap generator", QueueType.graphics)
end

function compute_motion_blur(input_target, output_target)
    render_graph:push_debug_marker("Compute motion blur", 0xffff33ff, QueueType.graphics)
	-- Compute static motion vectors (can be started in another queue)
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, use_low_precision_velocity and "postprocess:generate_motion_vectors:LOW_PRECISION_VELOCITY" or "postprocess:generate_motion_vectors")
        pass_ci:add_workgroup_size_by_rt("zbuffer_depth_target", true)
    
        pass_ci:bind_write_resource("write_color", "motion_vectors_target")
        pass_ci:bind_read_resource("depth_texture", "zbuffer_depth_target")
    
        render_graph:add_render_pass("generate_motion_vectors_pass", pass_ci)
    end
    
    -- Apply motion blur
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, use_low_precision_velocity and "postprocess:apply_effect:MOTION_BLUR:LOW_PRECISION_VELOCITY" or "postprocess:apply_effect:MOTION_BLUR")
        pass_ci:add_workgroup_size_by_rt(input_target, true)
    
        pass_ci:bind_read_resource("sampler_color", input_target)
        pass_ci:bind_read_resource("motion_vectors", "motion_vectors_target")
        pass_ci:bind_write_resource("write_color", output_target)
    
        render_graph:add_render_pass("apply_motion_blur", pass_ci)
    end

    render_graph:pop_debug_marker("Compute motion blur", QueueType.graphics)
end

function compute_dof(input_target, output_target)
    render_graph:push_debug_marker("Compute depth of field", 0xffff33ff, QueueType.graphics)

	do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:bokeh")
        pass_ci:add_workgroup_size_by_rt(input_target, true)
    
        pass_ci:bind_write_resource("write_color", "bokeh_target")
        pass_ci:bind_read_resource("sampler_color", input_target)
    
        render_graph:add_render_pass("compute_bokeh_blur_pass", pass_ci)
    end
    
    -- Apply dof
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:apply_effect:DOF")
        pass_ci:add_workgroup_size_by_rt(input_target, true)
    
        pass_ci:bind_write_resource("write_color", output_target)
        pass_ci:bind_read_resource("sampler_color", input_target)
        pass_ci:bind_read_resource("depth_image", "zbuffer_depth_target")
        pass_ci:bind_read_resource("bokeh_color", "bokeh_target")
    
        render_graph:add_render_pass("compute_bokeh_blur_pass", pass_ci)
    end

    render_graph:pop_debug_marker("Compute depth of field", QueueType.graphics)
end


function compute_bloom(input_target, output_target)
    render_graph:push_debug_marker("Compute bloom", 0xffff33ff, QueueType.graphics)

    -- index 10
    -- Starting from brightness detection pass
    do 
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:brightdetect")

        pass_ci:add_workgroup_size_by_rt("bloom_target", true)

        pass_ci:bind_read_resource("sampler_color", input_target)
        pass_ci:bind_write_resource("write_color", "brightness_target")

        render_graph:add_render_pass("brightdetect_pass", pass_ci)
    end

    -- Begin downsampling
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:downsample:FIRST_PASS")
        pass_ci:bind_read_resource("sampler_color", "brightness_target")

        render_graph:push_debug_marker("Downsample", 0xffff33ff, QueueType.graphics)
        for idx = 0, bloom_blur_depth - 1 do
            pass_ci:add_workgroup_size_by_rt("bloom_target", idx, true)
            pass_ci:bind_write_resource("write_color", "bloom_target", idx)
            
            render_graph:add_render_pass("downsample_pass_" .. idx, pass_ci)

            pass_ci:clear_resources()
            pass_ci:bind_read_resource("sampler_color", "bloom_target", idx)
            pass_ci.shader = "postprocess:downsample"
        end
        render_graph:pop_debug_marker("Downsample", QueueType.graphics)
    end

    -- And upsampling
    render_graph:push_debug_marker("Upsample", 0xffff33ff, QueueType.graphics)
    for idx = bloom_blur_depth - 1, 1, -1 do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:upsample")
    
        pass_ci:add_workgroup_size_by_rt("bloom_target", idx - 1, true)
    
        pass_ci:bind_read_resource("sampler_color", "bloom_target", idx)
        pass_ci:bind_write_resource("write_color", "bloom_target", idx - 1)
    
        render_graph:add_render_pass("upnsample_pass_" .. idx, pass_ci)
    end
    render_graph:pop_debug_marker("Upsample", QueueType.graphics)

    -- Apply bloom
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:apply_effect:BLOOM")
        pass_ci:add_workgroup_size_by_rt(output_target, true)

        pass_ci:bind_write_resource("write_color", output_target)
        pass_ci:bind_read_resource("sampler_color", input_target)
        pass_ci:bind_read_resource("bloom_color", "bloom_target", 0)

        render_graph:add_render_pass("apply_bloom", pass_ci)
    end

    render_graph:pop_debug_marker("Compute bloom", QueueType.graphics)
end

function compute_tonemap(input_target, output_target)
    render_graph:push_debug_marker("Compute tonemap", 0xffff33ff, QueueType.graphics)

    -- Compute histogram
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:compute_histogram")
        pass_ci:add_workgroup_size_by_rt(input_target, true)
    
        pass_ci:bind_read_resource("sampler_color", input_target)
        pass_ci:bind_write_resource("Histogram", "histogram_buffer")
    
        render_graph:add_render_pass("compute_histogram_pass", pass_ci)
    end
    
    -- Compute avg screen space brightness
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:compute_brightness")
        pass_ci:add_workgroup_size_manual(1, 1, 1, false)
    
        pass_ci:bind_write_resource("target", "avg_brightness_target")
        pass_ci:bind_read_resource("Histogram", "histogram_buffer")
    
        render_graph:add_render_pass("compute_brightness_pass", pass_ci)
    end

    -- Apply tonemapping
    do
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:apply_effect:TONEMAP")
        pass_ci:add_workgroup_size_by_rt(input_target, true)

        pass_ci:bind_write_resource("write_color", output_target)
        pass_ci:bind_read_resource("sampler_color", input_target)
        pass_ci:bind_read_resource("avg_luminosity", "avg_brightness_target")

        render_graph:add_render_pass("apply_bloom", pass_ci)
    end

    render_graph:pop_debug_marker("Compute tonemap", QueueType.graphics)
end

function postprocess(target_1, target_2)
	local input_target = target_1
    local output_target = target_2

    render_graph:push_debug_marker("Postprocess", 0xffff33ff, QueueType.graphics)
    -- Compute motion blur
    if motion_blur_enabled then
        compute_motion_blur(input_target, output_target)
        input_target, output_target = swap(input_target, output_target)
    end

    if depth_of_field_enabled then
        -- Compute bokeh
        compute_dof(input_target, output_target)
        input_target, output_target = swap(input_target, output_target)
    end

    -- Compute bloom
    if bloom_enabled then
        compute_bloom(input_target, output_target)
        input_target, output_target = swap(input_target, output_target)
    end

    -- Compute chromatic aberration
    if chromatic_aberration_enabled then
        local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "postprocess:apply_effect:CHROMATIC_ABERRATION")
        pass_ci:add_workgroup_size_by_rt(input_target, true)

        pass_ci:bind_write_resource("write_color", output_target)
        pass_ci:bind_read_resource("sampler_color", input_target)

        render_graph:add_render_pass("apply_chromatic_aberration_pass", pass_ci)
        input_target, output_target = swap(input_target, output_target)
    end

    do
        compute_tonemap(input_target, output_target)
        input_target, output_target = swap(input_target, output_target)
    end

    render_graph:pop_debug_marker("Postprocess", QueueType.graphics)

    return input_target
end



-- generate brdf lut
do
    local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "lighting:brgflut_generator")
    pass_ci.one_time = true

    pass_ci:add_workgroup_size_by_rt("brdf_lut_target", true)
    pass_ci:bind_write_resource("out_color", "brdf_lut_target")

    render_graph:add_render_pass("generate_brdflut", pass_ci)
end

do
    local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "lighting:irradiancecube_generator")
    pass_ci.one_time = true

    pass_ci:add_workgroup_size_by_rt("irradiance_cube_target", true)
    pass_ci:bind_write_resource("out_color", "irradiance_cube_target")
    pass_ci:bind_read_resource("sampler_color", "CasualDay4K")

    render_graph:add_render_pass("generate_irradiance_cube", pass_ci)
end

do
    local pass_ci = make_compute_pass(ComputeType.cubemap, QueueType.graphics, "lighting:prefilteredmap_generator")
    pass_ci.one_time = true

    pass_ci:add_workgroup_size_by_rt("prefiltered_cube_target", true)
    pass_ci:bind_write_resource("out_color", "prefiltered_cube_target")
    pass_ci:bind_read_resource("sampler_color", "CasualDay4K")

    render_graph:add_render_pass("generate_prefiltered_cube", pass_ci)
end

-- Culling (culling is temporary)
do
    local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "culling:clear_counters")
    pass_ci:add_workgroup_size_by_buffer("IndirectCountBuffer", true)
    pass_ci:add_write_resource("IndirectCountBuffer")

    render_graph:add_render_pass("clear_counters_pass", pass_ci)
end

do
    local pass_ci = make_compute_pass(ComputeType.culling, QueueType.graphics, "culling:cull_and_batch")

    pass_ci:add_write_resource("GeometryBatches")
    pass_ci:add_write_resource("IndexedDrawIndirectBuffer")
    pass_ci:add_write_resource("IndirectCountBuffer")

    render_graph:add_render_pass("gpu_culling_pass", pass_ci)
end

-- Shadow passes
do
    local pass_ci = make_graphics_pass(RenderingType.csm, QueueType.graphics, "lighting:unidirectional_shadow:CASCADED_SHADOW")
    pass_ci:set_depth_target("cascaded_shadows_target")

    pass_ci:add_read_resource("GeometryBatches")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")
    pass_ci:add_read_resource("IndirectCountBuffer") 

    render_graph:add_render_pass("cascaded_shadow_map_pass", pass_ci)
end

do
    local pass_ci = make_graphics_pass(RenderingType.dsm, QueueType.graphics, "lighting:unidirectional_shadow")
    pass_ci:set_depth_target("directional_shadows_target")

    pass_ci:add_read_resource("GeometryBatches")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")
    pass_ci:add_read_resource("IndirectCountBuffer")

    render_graph:add_render_pass("spot_shadow_map_pass", pass_ci)
end

do
    local pass_ci = make_graphics_pass(RenderingType.osm, QueueType.graphics, "lighting:unidirectional_shadow:OMNI_SHADOW")
    local pass_ci = make_graphics_pass(RenderingType.osm, QueueType.graphics, "lighting:unidirectional_shadow:OMNI_SHADOW")
    pass_ci:set_depth_target("omni_shadows_target")

    pass_ci:add_read_resource("GeometryBatches")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")
    pass_ci:add_read_resource("IndirectCountBuffer")

    render_graph:add_render_pass("omni_shadow_map_pass", pass_ci)
end

-- visibility buffer write
do
    local pass_ci = make_graphics_pass(RenderingType.geometry, QueueType.graphics, "visibilitybuffer:write")
    pass_ci:set_depth_target("zbuffer_depth_target")
    pass_ci:set_stencil_target("zbuffer_depth_target")

    pass_ci:add_write_resource("visbuffer_geometry_target", make_vec4(0.0, 0.0, 0.0, 0.0))
    pass_ci:add_write_resource("visbuffer_vboinst_target", make_vec4(0.0, 0.0, 0.0, 0.0))
    --pass_ci:add_write_resource("motion_vectors_target")
    pass_ci:add_read_resource("GeometryBatches")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")
    pass_ci:add_read_resource("IndirectCountBuffer")

    render_graph:add_render_pass("visibility_buffer_write_pass", pass_ci)
end

-- visibility buffer material pass
do
    local pass_ci = make_compute_pass(ComputeType.geometry, QueueType.graphics, "visibilitybuffer:shade")
    pass_ci:add_workgroup_size_by_rt("hdr0", true)

    pass_ci:bind_write_resource("write_color", "hdr0")
    pass_ci:bind_read_resource("prev_frame_color", "hdr0_history")
    pass_ci:bind_read_resource("visbuffer_geometry", "visbuffer_geometry_target")
    pass_ci:bind_read_resource("visbuffer_vboinst", "visbuffer_vboinst_target")
    pass_ci:bind_read_resource("brdf_lut", "brdf_lut_target")
    pass_ci:bind_read_resource("irradiance_cube", "irradiance_cube_target")
    pass_ci:bind_read_resource("prefiltered_cube", "prefiltered_cube_target")
    pass_ci:bind_read_resource("cascaded_shadowmap", "cascaded_shadows_target")
    pass_ci:bind_read_resource("direct_shadowmap", "directional_shadows_target")
    pass_ci:bind_read_resource("omni_shadowmap", "omni_shadows_target")
    pass_ci:bind_read_resource("depth_buffer", "zbuffer_depth_target")

    pass_ci:add_read_resource("GeometryBatches")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")

    render_graph:add_render_pass("compute_materials_pass", pass_ci)
end

do
    local pass_ci = make_compute_pass(ComputeType.default, QueueType.graphics, "utility:copy_compute")
    pass_ci:add_workgroup_size_by_rt("hdr0", true)

    pass_ci:bind_read_resource("sampler_color", "hdr0")
    pass_ci:bind_write_resource("write_color", "hdr0_history")

    render_graph:add_render_pass("copy_final_color_pass", pass_ci)
end

final_target = postprocess("hdr0", "hdr1")

-- Present pass
do
    local pass_ci = make_graphics_pass(RenderingType.fullscreen, QueueType.graphics, "testing:screenspace")
    pass_ci:flip_viewport()

    pass_ci:bind_read_resource("input_image", final_target)
    pass_ci:add_write_resource("present_target")

    render_graph:add_render_pass("finalize_image_pass", pass_ci)
end