#include "../include/atmosphere.slang"
#include "../include/bindles_layout.slang"
#include "../include/compute_common.slang"
#include "../include/lighting.slang"
#include "../include/packing.slang"
#include "../include/util.slang"

[[vk::binding(0, 1)]] ConstantBuffer<interop::EncodedGeometryData> egd : register(c0, space1);
[[vk::binding(1, 1)]] WTexture2D<float4> image0 : register(u0, space1);
[[vk::binding(2, 1)]] Texture2D texture0 : register(t0, space1); // BRDFlut
[[vk::binding(3, 1)]] TextureCube texture1 : register(t1, space1); // Irradiance cubemap
[[vk::binding(4, 1)]] TextureCube texture2 : register(t2, space1); // Prefiltered cubemap
[[vk::binding(5, 1)]] Texture2D texture3 : register(t3, space1); // Visibility buffer
[[vk::binding(6, 1)]] Texture2D texture4 : register(t4, space1); // Visibility buffer
[[vk::binding(7, 1)]] Texture2D texture5 : register(t5, space1); // depth

// For shadows
[[vk::binding(8, 1)]] Texture2DArray texture6 : register(t6, space1); // cascaded shadows
[[vk::binding(9, 1)]] Texture2DArray texture7 : register(t7, space1); // spot shadows
[[vk::binding(10, 1)]] TextureCubeArray texture8 : register(t8, space1); // omni shadows

[[vk::binding(11, 1)]] Texture2D texture9 : register(t9, space1); // ambient occlusion

[[vk::binding(12, 1)]] SamplerState sampler_point : register(s0, space1);
[[vk::binding(13, 1)]] SamplerState sampler_linear : register(s1, space1);
[[vk::binding(14, 1)]] SamplerState sampler_shadow_linear : register(s1, space1);

// #define TILE_MATERIAL_CLASSIFICATION
#define DISABLE_SHADOWS

struct VertexAttribute {
    float3 pos;
    float3 pos_dx;
    float3 pos_dy;

    float linearZ;

    float2 uv;
    float2 uv_dx;
    float2 uv_dy;

    float3 normal;
    float4 tangent;
    float4 color;

    float3 barycentrics;
};

struct BarycentricDeriv {
    float3 barycentrics;
    float3 ddx;
    float3 ddy;
};

struct VisibilityBuffer {
    uint draw_id;
    uint triangle_id;
    uint transparent;

    uint mat_id;
    uint vbo_id;
};

MaterialSample sample_material(in interop::Material mat, in VertexAttribute va) {
    MaterialSample res;

    // Sample albedo
    float4 base_color = mat.base_color_factor * va.color;
    if (mat.base_color.valid()) {
        base_color *= sample_texture_grad(mat.base_color, va.uv, va.uv_dx, va.uv_dy);
    }

    res.albedo = base_color.rgb;
    res.opacity = base_color.a;

    // Sample emission
    res.emission = mat.emissive_factor;
    if (mat.emissive.valid())
        res.emission *= sample_texture_grad(mat.emissive, va.uv, va.uv_dx, va.uv_dy).rgb;
    res.emission *= mat.emissive_strength;

    // Sample metallic roughness
    res.metallic = mat.metallic_factor;
    res.roughness = mat.roughness_factor;
    if (mat.metallic_roughness.valid()) {
        float4 mr = sample_texture_grad(mat.metallic_roughness, va.uv, va.uv_dx, va.uv_dy);
        res.metallic *= mr.b;
        res.roughness *= mr.g;
    }
    res.roughness = clamp(res.roughness, 0.04, 1.0);

    // Sample normals
    res.normal = va.normal;
    if (mat.normal.valid()) {
        float3 raw_normal = sample_texture_grad(mat.normal, va.uv, va.uv_dx, va.uv_dy).rgb;

        // Some textures can have only rg channels, we need to decode third
        if (raw_normal.b == 0) {
            raw_normal.rg = raw_normal.rg * 2.0 - 1.0;
            raw_normal.b = sqrt(clamp(1.0 - dot(raw_normal, raw_normal), 0.0, 1.0));
        }
        else {
            raw_normal = raw_normal * 2.0 - 1.0;
        }

        const float3 b = cross(va.normal, va.tangent.xyz) * va.tangent.w;
        const float3x3 tbn = float3x3(va.tangent.xyz, b, va.normal);

        const float3 ts_normal = normalize(raw_normal) * float3(mat.normal_scale, mat.normal_scale, 1.0);
        res.normal = normalize(mul(ts_normal, tbn));
    }

    // Sample AO
    res.ao = 1.0;
    if (mat.occlusion.valid()) {
        res.ao = sample_texture_grad(mat.occlusion, va.uv, va.uv_dx, va.uv_dy).r;
    }

    res.ior = mat.ior;

    return res;
}

float3 calculate_directional_light(in interop::LightCommit light, in VertexAttribute va, in MaterialSample ms, in float3 V, in float3 F0, in float dither) {
    float3 L = normalize(-light.direction);

    float shadow_factor = 1.0;
    if (light.cast_shadows > 0 && light.shadow_index > -1) {
        // shadow_factor = getCascadeShadow(texture6, light, va, ms, dither);
    }

    float3 color = specular_contribution(ms.albedo, L, V, ms.normal, F0, ms.metallic, ms.roughness);
    return light.color * light.intencity * color * shadow_factor;
}

float3 calculate_point_light(in interop::LightCommit light, in VertexAttribute va, in MaterialSample ms, in float3 V, in float3 F0, in float dither) {
    float3 L = normalize(light.position - va.pos);
    float dist = distance(va.pos, light.position);

    float atten = clamp(1.0 - pow(dist, 2.0f) / pow(light.radius, 2.0f), 0.0f, 1.0f); atten *= atten;
    float3 color = specular_contribution(ms.albedo, L, V, ms.normal, F0, ms.metallic, ms.roughness);

    float shadow_factor = 1.0;
    if (light.cast_shadows > 0 && light.shadow_index > -1) {
        //shadow_factor = getOmniShadow(texture8, light, va, ms, dither);
    }

    return light.color * atten * color * light.intencity * shadow_factor;
}

float3 calculate_spotlight(in interop::LightCommit light, in VertexAttribute va, in MaterialSample ms, in float3 V, in float3 F0, in float dither) {
    float3 L = normalize(light.position - va.pos);

    // Direction vector from source to target
    float3 dir = normalize(light.direction);
    if (light.to_target > 0)
        dir = normalize(light.position - light.direction);

    float3 color = specular_contribution(ms.albedo, L, V, ms.normal, F0, ms.metallic, ms.roughness);

    // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/README.md
    float cd = dot(light.to_target > 0 ? L : -L, dir);
    float angularAttenuation = clamp(cd * light.light_angle_scale + light.light_angle_offset, 0.0, 1.0);
    angularAttenuation *= angularAttenuation;

    float shadow_factor = 1.0;
    if (light.cast_shadows > 0 && light.shadow_index > -1) {
        //shadow_factor = getDirectionalShadow(texture7, light, va, ms, dither);
    }

    return light.color * light.intencity * color * angularAttenuation * shadow_factor;
}

float3 calculate_lights(in VertexAttribute va, in MaterialSample ms, in float3 V, in float3 F0, in float dither) {
    float3 Lo = float3(0.0);

    for (uint i = 0u; i < g_fd.light_count; i++) {
        interop::LightCommit light = g_fd.lights_ptr[i];

        if (light.type == interop::LightType::eDirectional)
            Lo += calculate_directional_light(light, va, ms, V, F0, dither);
        else if (light.type == interop::LightType::ePoint)
            Lo += calculate_point_light(light, va, ms, V, F0, dither);
        else if (light.type == interop::LightType::eSpot)
            Lo += calculate_spotlight(light, va, ms, V, F0, dither);
    }

    return Lo;
}

inline bool load_vis_buffer(in float4 vb0, in float4 vb1, out VisibilityBuffer res) {
    uint32_t geometry_data = rgba8_unorm::pack(vb0);

    res.draw_id = (geometry_data >> 23) & 0x000000FF;
    res.triangle_id = (geometry_data & 0x007FFFFF);
    res.transparent = (geometry_data >> 31);
    res.triangle_id -= 1u;

    if (res.triangle_id == 0xFFFFFFFFu) {
        return false;
    }

    uint vboinst_data = rgba8_unorm::pack(vb1);
    res.vbo_id = vboinst_data & 0xFFFF;
    uint inst_id = vboinst_data >> 16;

    uint32_t draw_index = res.draw_id + inst_id;
    interop::DrawDescription draw_info = egd.draw_descriptions_ptr[draw_index];
    interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
    interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];

    res.mat_id = submesh.material;

    return true;
}

// Computes the partial derivatives of a triangle from the projected screen space vertices
// Based on: http://filmicworlds.com/blog/visibility-buffer-rendering-with-material-graphs/
BarycentricDeriv compute_barycentrics(float4 vertexCS0, float4 vertexCS1, float4 vertexCS2, float2 pixelCS, float2 invSize) {
    BarycentricDeriv ret;

    float3 invW = 1.0f / float3(vertexCS0.w, vertexCS1.w, vertexCS2.w);

    float2 ndc0 = vertexCS0.xy * invW.x;
    float2 ndc1 = vertexCS1.xy * invW.y;
    float2 ndc2 = vertexCS2.xy * invW.z;

    float invDet = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));
    ret.ddx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
    ret.ddy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;
    float ddxSum = dot(ret.ddx, float3(1.0, 1.0, 1.0));
    float ddySum = dot(ret.ddy, float3(1.0, 1.0, 1.0));

    float2 deltaVec = pixelCS - ndc0;
    float interpInvW = invW.x + deltaVec.x * ddxSum + deltaVec.y * ddySum;
    float interpW = 1.0 / interpInvW;

    ret.barycentrics.x = interpW * (invW[0] + deltaVec.x * ret.ddx.x + deltaVec.y * ret.ddy.x);
    ret.barycentrics.y = interpW * (0.0f + deltaVec.x * ret.ddx.y + deltaVec.y * ret.ddy.y);
    ret.barycentrics.z = interpW * (0.0f + deltaVec.x * ret.ddx.z + deltaVec.y * ret.ddy.z);

    ret.ddx *= (2.0f * invSize.x);
    ret.ddy *= (2.0f * invSize.y);
    ddxSum *= (2.0f * invSize.x);
    ddySum *= (2.0f * invSize.y);

    float interpW_ddx = 1.0f / (interpInvW + ddxSum);
    float interpW_ddy = 1.0f / (interpInvW + ddySum);

    ret.ddx = interpW_ddx * (ret.barycentrics * interpInvW + ret.ddx) - ret.barycentrics;
    ret.ddy = interpW_ddy * (ret.barycentrics * interpInvW + ret.ddy) - ret.barycentrics;

    return ret;
}

inline float interpolate(float a, float b, float c, float3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

inline float2 interpolate(float2 a, float2 b, float2 c, float3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

inline float3 interpolate(float3 a, float3 b, float3 c, float3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

inline float4 interpolate(float4 a, float4 b, float4 c, float3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

[numthreads(WG_TILE_SIZE, WG_TILE_SIZE, 1)]
[shader("compute")]
void main(uint3 global_id: SV_DispatchThreadID, uint3 local_id: SV_GroupThreadID, uint3 group_id: SV_GroupID) {

    uint32_t2 image_size;
    image0.GetDimensions(image_size.x, image_size.y);
    float2 texel_size = 1.0f / float2(image_size);

    uint32_t2 px;
    float2 uv;

    [branch]
    if (swizzle_reject_workgroups(global_id.xy, local_id.xy, group_id.xy, image_size, uv, px)) {
        return;
    }

    // TODO: set camera from another place
    interop::Camera camera = g_fd.get_default_camera();
    float4x4 inv_view = g_fd.get_inv_view_matrix(camera.matrices_offset);

    float3 camera_pos = float3(inv_view[0][3], inv_view[1][3], inv_view[2][3]);

    // float4 pixel_color = float4(0.35f, 0.55f, 0.7f, 1.0f);
    float4 pixel_color = float4(0.0f, 0.0f, 0.0f, 1.0f);

    // ----------------- Load visibility buffer -----------------
    VisibilityBuffer vb;
    float4 vb0 = texture3.SampleLevel(sampler_point, uv, 0.0f);
    float4 vb1 = texture4.SampleLevel(sampler_point, uv, 0.0f);

    uint32_t geometry_data = rgba8_unorm::pack(vb0);
    vb.draw_id = (geometry_data >> 23) & 0x000000FF;
    vb.triangle_id = (geometry_data & 0x007FFFFF);
    vb.transparent = (geometry_data >> 31);
    vb.triangle_id -= 1u;

    [branch]
    if (vb.triangle_id == 0xFFFFFFFFu) {
        // TODO: will work after adding lights
        float4 dir_world_space = float4(0.0);
        const float3 origin = float3(0, atmosphere::k_planet_radius, 0) + camera_pos;

        float depth = texture5.SampleLevel(sampler_point, uv, 0.0f).r;
        dir_world_space.xyz = util::position_from_depth(uv, 1.0f, g_fd.get_inv_view_projection_matrix(camera.matrices_offset));

        uint sun_count = 0;
        for (int i = 0; i < g_fd.light_count; i++) {
            const interop::LightCommit light = g_fd.lights_ptr[i];
            if (light.type == interop::LightType::eDirectional) {
                const float3 dir_to_sun = normalize(-light.direction);

                pixel_color.rgb += atmosphere::scatter(origin, dir_world_space.xyz, -dir_to_sun, light.intencity);
                //pixel_color.rgb += atmosphere::scatter(origin, dir_world_space.xyz, dir_to_sun, light.intencity);

                sun_count++;
                break;
            }
        }

        pixel_color.rgb /= float(sun_count);
        pixel_color.rgb = 1.0 - exp(-1.0 * pixel_color.rgb);

        image0.Store(px, pixel_color);
        return;
    }

    uint32_t vboinst_data = rgba8_unorm::pack(vb1);
    vb.vbo_id = vboinst_data & 0xFFFF;
    uint inst_id = vboinst_data >> 16;

    uint32_t draw_index = vb.draw_id + inst_id;
    interop::DrawDescription draw_info = egd.draw_descriptions_ptr[draw_index];
    interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
    interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];
    vb.mat_id = submesh.material;

    // ----------------- Load vertex atribure data -----------------
    VertexAttribute va;
    {
        interop::DecodedTriangle triangle = egd.get_dec_triangle(vb.draw_id, vb.triangle_id, mesh.uv_enc);

        // Load vertex data of the 3 vertices
        float3 position_0 = triangle.verts[0].position;
        float3 position_1 = triangle.verts[1].position;
        float3 position_2 = triangle.verts[2].position;

        // Load mesh model matrix

        float4x4 model = g_fd.get_model_matrix(mesh.matrix_offset);
        float4x4 normal_matrix = g_fd.get_normal_matrix(mesh.matrix_offset);

        interop::Camera camera = g_fd.get_default_camera();
        float4x4 view_projection = g_fd.get_view_projection_matrix(camera.matrices_offset);

        float4 clip_pos_0;
        float4 clip_pos_1;
        float4 clip_pos_2;

#ifdef HAS_SKIN
        if (mesh.skin_offset != ~0u) {
            joints_t joints0 = triangle.verts[0].joints;
            joints_t joints1 = triangle.verts[1].joints;
            joints_t joints2 = triangle.verts[2].joints;

            float4 weights0 = triangle.verts[0].weights;
            float4 weights1 = triangle.verts[1].weights;
            float4 weights2 = triangle.verts[2].weights;

            float4x4 skin_mat0 = 
			                 weights0.x * g_fd.material_ptr[mesh.skin_offset + joints0.x] +
                             weights0.y * g_fd.material_ptr[mesh.skin_offset + joints0.y] +
                             weights0.z * g_fd.material_ptr[mesh.skin_offset + joints0.z] +
                             weights0.w * g_fd.material_ptr[mesh.skin_offset + joints0.w];

            float4x4 skin_mat1 = 
			                 weights1.x * g_fd.material_ptr[mesh.skin_offset + joints1.x] +
                             weights1.y * g_fd.material_ptr[mesh.skin_offset + joints1.y] +
                             weights1.z * g_fd.material_ptr[mesh.skin_offset + joints1.z] +
                             weights1.w * g_fd.material_ptr[mesh.skin_offset + joints1.w];

            float4x4 skin_mat2 = 
			                 weights2.x * g_fd.material_ptr[mesh.skin_offset + joints2.x] +
                             weights2.y * g_fd.material_ptr[mesh.skin_offset + joints2.y] +
                             weights2.z * g_fd.material_ptr[mesh.skin_offset + joints2.z] +
                             weights2.w * g_fd.material_ptr[mesh.skin_offset + joints2.w];

            float4x4 mvp0 = mul(mul(view_projection, model), skin_mat0);
            float4x4 mvp1 = mul(mul(view_projection, model), skin_mat1);
            float4x4 mvp2 = mul(mul(view_projection, model), skin_mat2);

            // Transform positions to clip space
            clip_pos_0 = mul(mvp0, float4(position0, 1.0));
            clip_pos_1 = mul(mvp1, float4(position1, 1.0));
            clip_pos_2 = mul(mvp2, float4(position2, 1.0));
        }
#else
        // Transform positions to clip space
        float4x4 mvp = mul(view_projection, model);
        clip_pos_0 = mul(mvp, float4(position_0, 1.0));
        clip_pos_1 = mul(mvp, float4(position_1, 1.0));
        clip_pos_2 = mul(mvp, float4(position_2, 1.0));
#endif // HAS_SKIN

        // uv.y = 1.0 - uv.y;
        float2 ndc_clip = util::uv_to_clip(uv);
        BarycentricDeriv bary = compute_barycentrics(clip_pos_0, clip_pos_1, clip_pos_2, ndc_clip, camera.inv_size);

        // INTERPOLATE POSITION
        // TODO: Skinning. Here i have to multiply to specific matrices
        position_0 = mul(model, float4(position_0, 1.0)).xyz;
        position_1 = mul(model, float4(position_1, 1.0)).xyz;
        position_2 = mul(model, float4(position_2, 1.0)).xyz;

        va.pos = interpolate(position_0, position_1, position_2, bary.barycentrics);
        va.pos_dx = interpolate(position_0, position_1, position_2, bary.ddx);
        va.pos_dy = interpolate(position_0, position_1, position_2, bary.ddy);

        va.linearZ = interpolate(clip_pos_0.w, clip_pos_1.w, clip_pos_2.w, bary.barycentrics);
        va.barycentrics = bary.barycentrics;

        // INTERPOLATE TEXCOORD
#ifdef HAS_TEXCOORD
        float2 uv0 = triangle.verts[0].uv;
        float2 uv1 = triangle.verts[1].uv;
        float2 uv2 = triangle.verts[2].uv;

        va.uv = interpolate(uv0, uv1, uv2, bary.barycentrics);
        va.uv_dx = interpolate(uv0, uv1, uv2, bary.ddx);
        va.uv_dy = interpolate(uv0, uv1, uv2, bary.ddy);
#endif

// INTERPOLATE NORMAL
#ifdef HAS_NORMAL
        float3 normal_0 = normalize(mul(normal_matrix, float4(triangle.verts[0].normal, 0.0f)).xyz);
        float3 normal_1 = normalize(mul(normal_matrix, float4(triangle.verts[1].normal, 0.0f)).xyz);
        float3 normal_2 = normalize(mul(normal_matrix, float4(triangle.verts[2].normal, 0.0f)).xyz);

        va.normal = normalize(interpolate(normal_0, normal_1, normal_2, bary.barycentrics));
#else
        va.normal = cross(va.pos_dx, va.pos_dy);
#endif // HAS_NORMAL

// INTERPOLATE TANGENT
#ifdef HAS_TANGENT
        float4 tangent0 = float4(normalize(mul(model, float4(triangle.verts[0].tangent.xyz, 0.0f)).xyz), triangle.verts[0].tangent.w);
        float4 tangent1 = float4(normalize(mul(model, float4(triangle.verts[1].tangent.xyz, 0.0f)).xyz), triangle.verts[1].tangent.w);
        float4 tangent2 = float4(normalize(mul(model, float4(triangle.verts[2].tangent.xyz, 0.0f)).xyz), triangle.verts[2].tangent.w);

        float4 tangent = interpolate(tangent0, tangent1, tangent2, bary.barycentrics);
        va.tangent = float4(normalize(tangent.xyz), tangent.w);
#else
        float3 tangent = (va.uv_dy.y * va.pos_dx - va.uv_dx.y * va.pos_dy) / (va.uv_dx.x * va.uv_dy.y - va.uv_dy.x * va.uv_dx.y);
        tangent = normalize(tangent - va.normal * dot(va.normal, tangent));
        va.tangent = float4(tangent, 1.0);
#endif // TANGENT

        // INTERPOLATE COLOR
#ifdef HAS_COLOR
        float4 color_0 = triangle.verts[0].color;
        float4 color_1 = triangle.verts[1].color;
        float4 color_2 = triangle.verts[2].color;

        va.color = interpolate(color_0, color_1, color_2, bary.barycentrics);

#else
        va.color = float4(1.0);
#endif // COLOR
    }

    // TODO
    float dither = 0.f;

    interop::Material material = g_fd.material_ptr[vb.mat_id];
    MaterialSample ms = sample_material(material, va);

    // Calculate direction from fragment to viewPosition
    float3 V = normalize(camera_pos - va.pos);

    float3 F0 = f0_from_ior(ms.ior);
    F0 = lerp(F0, ms.albedo, ms.metallic);

    // TODO: finish light calculation
    float3 Lo = calculate_lights(va, ms, V, F0, dither);

    // Reflection vector
    float3 R = -normalize(reflect(V, ms.normal));

    float NdotV = dot(ms.normal, V);
    float2 brdf = texture0.SampleLevel(sampler_point, float2(max(NdotV, 0.0f), ms.roughness), 0.0f).xy;
    float3 reflection = prefiltered_reflection(R, ms.roughness, texture2, sampler_linear);
    float3 irradiance = texture1.SampleLevel(sampler_linear, ms.normal, 0.0f).xyz;

    // Diffuse based on irradiance
    float3 diffuse = ms.albedo * irradiance;

    float3 F = f_schlick_r(max(NdotV, 0.0f), F0, ms.roughness);

    // Specular reflectance
    float3 specular = reflection * (F * brdf.x + brdf.y);

    float3 kD = (1.0 - F) * (1.0 - ms.metallic);
    float3 ambient = kD * diffuse + specular;

    ambient = lerp(ambient, ambient * ms.ao * texture9.SampleLevel(sampler_linear, uv, 0.0f).r, material.occlusion_strenth);

    float3 res = ambient + ms.emission + Lo;
    // res *= debug_visualize_cascades(va.linearZ, dither);

    pixel_color = float4(res, 1.0);
    // pixel_color.rgb += computeSSR(uv, va.pos, va.normal, ms.metallic, ms.roughness, sampler2D(texture5, sampler_point), prev_frame_color);
    float ssr_weight = 0.f;
    // pixel_color.rgb += ScreenSpaceReflections(va.pos, ms.normal, V, ms.roughness, sampler2D(texture5, sampler_point), prev_frame_color, dither, ssr_weight);

    image0.Store(px, pixel_color);
}