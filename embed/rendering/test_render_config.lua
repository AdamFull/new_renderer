-- Loading test cubemap
do
    render_graph:add_static_resource("CasualDay4K", "/root/CasualDay4K.ktx2")
end

-- GI targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rg16_sfloat)
    target_ci.type = texture_or(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_fixed_size_2d(1024)

    render_graph:add_render_resource("brdf_lut_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba32_sfloat)
    target_ci.type = texture_or(TexType.two_dim, TexType.cubemap, TexType.samplable, TexType.storage)
    target_ci:set_fixed_size_2d(64)

    render_graph:add_render_resource("irradiance_cube_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba16_sfloat)
    target_ci.type = texture_or(TexType.two_dim, TexType.cubemap, TexType.samplable, TexType.storage)
    target_ci:set_fixed_size_2d(512)
    target_ci:enable_mip_map()
    
   render_graph:add_render_resource("prefiltered_cube_target", target_ci)
end

-- Shadow targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(false)
    target_ci.type = texture_or(TexType.two_dim, TexType.array, TexType.samplable)
    target_ci:set_fixed_size_2d(global_settings.cascade_shadowmap_resolution)
    target_ci.array_layers = global_settings.cascade_shadowmap_cascade_count * global_settings.max_directional_light_shadow_count

    render_graph:add_render_resource("cascaded_shadows_target", target_ci)
end

-- Visibility buffer targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type = texture_or(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_geometry_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type = texture_or(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_matobj_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba8_unorm)
    target_ci.type = texture_or(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)
    
    render_graph:add_render_resource("visbuffer_skinvbo_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_depth_format(false)
    target_ci.type = texture_or(TexType.two_dim, TexType.samplable)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("zbuffer_depth_target", target_ci)
end

do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_api_format(Format.rgba16_sfloat)
    target_ci.type = texture_or(TexType.two_dim, TexType.samplable, TexType.storage)
    target_ci:set_size_scale(1.0)

    render_graph:add_render_resource("final_lighting_target", target_ci)
end

-- Present image targets
do
    local target_ci = TextureCI.new()
    target_ci.format = render_context:get_present_format()
    target_ci.type = texture_or(TexType.two_dim)
    target_ci:use_as_backbuffer()

    render_graph:add_render_resource("present_target", target_ci)
end








-- generate brdf lut
do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.one_time = true
    pass_ci.type = ComputeType.default
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "lighting:brgflut_generator";

    pass_ci.size_target = "brdf_lut_target";
    pass_ci:bind_write_resource("out_color", "brdf_lut_target")

    render_graph:add_render_pass("generate_brdflut", pass_ci)
end

do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.one_time = true
    pass_ci.type = ComputeType.default
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "lighting:irradiancecube_generator";

    pass_ci.size_target = "irradiance_cube_target";
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

    pass_ci:bind_write_resource("out_color", "prefiltered_cube_target")
    pass_ci:bind_read_resource("sampler_color", "CasualDay4K")

    render_graph:add_render_pass("generate_prefiltered_cube", pass_ci)
end


--do
--    local pass_ci = ComputeCreateInfo.new()
--    pass_ci.type = ComputeType.culling
--    pass_ci.queue = QueueType.graphics
--    pass_ci.shader = "culling:cull_and_batch"
--
--    pass_ci:bind_read_resource("nodeBuffer", "NodeStorage")
--    pass_ci:bind_read_resource("frustumSides", "FrustumSides")
--    pass_ci:bind_read_resource("cullingBuffer", "GeometryCullingInfo")
--    pass_ci:bind_write_resource("indirectCommandBuffer", "IndexedDrawIndirectBuffer")
--
--    render_graph:add_render_pass("gpu_culling", pass_ci)
--end


do
    local pass_ci = GraphicsCreateInfo.new()
    pass_ci.type = RenderingType.csm
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "lighting:unidirectional_shadow:CASCADED_SHADOW"

    pass_ci.size_target = "cascaded_shadows_target"
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

    pass_ci.size_target = "zbuffer_depth_target"
    pass_ci:set_depth_target("zbuffer_depth_target")
    pass_ci:set_stencil_target("zbuffer_depth_target")

    pass_ci:bind_read_resource("nodeBuffer", "NodeStorage")
    pass_ci:bind_read_resource("jointsBuffer", "SkinningJoints")

    pass_ci:add_write_resource("visbuffer_geometry_target")
    pass_ci:add_write_resource("visbuffer_matobj_target")
    pass_ci:add_write_resource("visbuffer_skinvbo_target")
    pass_ci:add_read_resource("IndexedDrawIndirectBuffer")

    render_graph:add_render_pass("visibility_buffer_write_pass", pass_ci)
end

-- visibility buffer material pass
do
    local pass_ci = ComputeCreateInfo.new()
    pass_ci.type = ComputeType.geometry
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "visibilitybuffer:shade"

    pass_ci.size_target = "final_lighting_target"

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

-- Present pass
do
    local pass_ci = GraphicsCreateInfo.new()
    pass_ci.type = RenderingType.fullscreen
    pass_ci.queue = QueueType.graphics
    pass_ci.shader = "testing:screenspace"
    pass_ci.flip_viewport = true

    pass_ci.size_target = "present_target"
    pass_ci:bind_read_resource("input_image", "final_lighting_target")
    pass_ci:add_write_resource("present_target")

    render_graph:add_render_pass("finalize_image_pass", pass_ci)
end