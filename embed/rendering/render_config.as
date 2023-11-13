namespace global_settings
{
    uint max_directional_light_count = 3;
    uint max_directional_light_shadow_count = 1;
    uint cascade_shadowmap_resolution = 2048;
    uint cascade_shadowmap_cascade_count = 8;

    uint max_spot_light_count = 64;
    uint max_spot_light_shadow_count = 16;
    uint spotlight_shadowmap_resolution = 1024;

    uint max_point_light_count = 64;
    uint max_point_light_shadow_count = 16;
    uint point_light_shadowmap_resolution = 512;
}

void init_render_targets()
{
    FU32Point2D render_resolution = render_context.getRenderResolution();

    // Other targets
    {
        // TODO: it should be storage target
        FTextureCreateInfo target_ci = FTextureCreateInfo();
        target_ci.format = render_context.getApiFormat(FORMAT_RG16_SFLOAT);
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT;
        target_ci.width = target_ci.height = 1024;

        render_context.createRenderTarget("brdf_lut_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getApiFormat(FORMAT_RGBA32_SFLOAT);
        target_ci.type = TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT;
        target_ci.width = target_ci.height = 64;

        render_context.createRenderTarget("irradiance_cube_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getApiFormat(FORMAT_RGBA16_SFLOAT);
        target_ci.type = TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT;
        target_ci.width = target_ci.height = 512;

        render_context.createRenderTarget("prefiltered_cube_target", target_ci);
    }

    // Shadowmap render targets
    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getDepthFormat();
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT | TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = target_ci.height = global_settings::cascade_shadowmap_resolution;
        target_ci.array_layers = global_settings::cascade_shadowmap_cascade_count * global_settings::max_directional_light_shadow_count;

        render_context.createRenderTarget("cascaded_sm_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getDepthFormat();
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT | TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = target_ci.height = global_settings::spotlight_shadowmap_resolution;
        target_ci.array_layers = global_settings::max_spot_light_shadow_count;

        render_context.createRenderTarget("directional_sm_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getDepthFormat();
        target_ci.type = TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT | TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT | TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = target_ci.height = global_settings::point_light_shadowmap_resolution;
        target_ci.array_layers = global_settings::max_point_light_shadow_count;

        render_context.createRenderTarget("omni_sm_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getDepthFormat();
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = render_resolution.x;
        target_ci.height = render_resolution.y;

        render_context.createRenderTarget("shadowmap_composition_target", target_ci);
    }

    // Z-Buffer targets
    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getApiFormat(FORMAT_RGBA8_SRGB);
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = render_resolution.x;
        target_ci.height = render_resolution.y;

        render_context.createRenderTarget("zbuffer_albedo_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getApiFormat(FORMAT_RGBA16_SFLOAT);
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = render_resolution.x;
        target_ci.height = render_resolution.y;

        render_context.createRenderTarget("zbuffer_normal_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getApiFormat(FORMAT_RGBA8_UNORM);
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = render_resolution.x;
        target_ci.height = render_resolution.y;

        render_context.createRenderTarget("zbuffer_pbr_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getApiFormat(FORMAT_RGBA8_SRGB);
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = render_resolution.x;
        target_ci.height = render_resolution.y;

        render_context.createRenderTarget("zbuffer_emission_target", target_ci);
    }

    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getDepthFormat();
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
        target_ci.width = render_resolution.x;
        target_ci.height = render_resolution.y;

        render_context.createRenderTarget("zbuffer_depth_target", target_ci);
    }

    // Present image targets
    {
        FTextureCreateInfo target_ci;
        target_ci.format = render_context.getPresentFormat();
        target_ci.type = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT;
        target_ci.width = render_resolution.x;
        target_ci.height = render_resolution.y;
        target_ci.present_image = true;

        render_context.createRenderTarget("present_target", target_ci);
    }
}

void init_render_passes()
{
    // Other passes
    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_BRDFLUT_GENERATOR;
        rendering_ci.shader = render_interface.getShaderNameTag("generators:brdflut_generator");
        rendering_ci.output.push_back(render_context.getResource("brdf_lut_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.one_time = true;

        render_interface.createRendering(rendering_ci);
    }

    // Cubemap generation
    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_IRRADIANCE_CUBE_GENERATOR;
        rendering_ci.shader = render_interface.getShaderNameTag("generators:irradiancecube_generator");
        rendering_ci.output.push_back(render_context.getResource("irradiance_cube_target", RESOURCE_TYPE_RENDER_TARGET_BIT));

        render_interface.createRendering(rendering_ci);
    }

    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_PREFILTERED_CUBE_GENERATOR;
        rendering_ci.shader = render_interface.getShaderNameTag("generators:prefilteredmap_generator");
        rendering_ci.output.push_back(render_context.getResource("prefiltered_cube_target", RESOURCE_TYPE_RENDER_TARGET_BIT));

        render_interface.createRendering(rendering_ci);
    }

    // Render shadows
    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_CASCADED_SHADOWS;
        rendering_ci.shader = render_interface.getShaderNameTag("shadows:cascaded");
        rendering_ci.depth = render_context.getResource("cascaded_sm_target", RESOURCE_TYPE_RENDER_TARGET_BIT);

        render_interface.createRendering(rendering_ci);
    }

    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_DIRECTIONAL_SHADOWS;
        rendering_ci.shader = render_interface.getShaderNameTag("shadows:directional");
        rendering_ci.depth = render_context.getResource("directional_sm_target", RESOURCE_TYPE_RENDER_TARGET_BIT);

        render_interface.createRendering(rendering_ci);
    }

    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_OMNI_SHADOWS;
        rendering_ci.shader = render_interface.getShaderNameTag("shadows:omni");
        rendering_ci.depth = render_context.getResource("omni_sm_target", RESOURCE_TYPE_RENDER_TARGET_BIT);

        render_interface.createRendering(rendering_ci);
    }

    // Compose shadowmaps in one target (shadowmap projection)
    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_COMPUTE;
        rendering_ci.shader = render_interface.getShaderNameTag("shadows:projection");
        rendering_ci.input.push_back(render_context.getResource("cascaded_sm_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.input.push_back(render_context.getResource("directional_sm_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.input.push_back(render_context.getResource("omni_sm_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.output.push_back(render_context.getResource("shadowmap_composition_target", RESOURCE_TYPE_RENDER_TARGET_BIT));

        render_interface.createRendering(rendering_ci);
    }

    // Z-buffer pass
    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_ZBUFFER;
        //rendering_ci.internal_control = true;
        rendering_ci.depth = render_context.getResource("zbuffer_depth_target", RESOURCE_TYPE_RENDER_TARGET_BIT);

        render_interface.createRendering(rendering_ci);
    }

    // Render image to final target
    {
        FRenderingStepCreateInfo rendering_ci;
        rendering_ci.queue = QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT;
        rendering_ci.type = RENDERING_TYPE_SCREENSPACE;
        rendering_ci.shader = render_interface.getShaderNameTag("lighting:lighting");
        rendering_ci.input.push_back(render_context.getResource("zbuffer_albedo_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.input.push_back(render_context.getResource("zbuffer_normal_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.input.push_back(render_context.getResource("zbuffer_pbr_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.input.push_back(render_context.getResource("zbuffer_emission_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.input.push_back(render_context.getResource("zbuffer_depth_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.input.push_back(render_context.getResource("shadowmap_composition_target", RESOURCE_TYPE_RENDER_TARGET_BIT));
        rendering_ci.output.push_back(render_context.getResource("present_target", RESOURCE_TYPE_RENDER_TARGET_BIT));

        render_interface.createRendering(rendering_ci);
    }
}

void main()
{
    init_render_targets();
    init_render_passes();
}