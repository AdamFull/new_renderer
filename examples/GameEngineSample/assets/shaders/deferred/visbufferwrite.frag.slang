#include "../include/bindles_layout.slang"

[[vk::binding(0, 1)]] ConstantBuffer<interop::EncodedGeometryData> egd : register(c0, space1);

struct FSInput {
    uint32_t draw_id : TEXCOORD0;
    uint32_t instance_id : TEXCOORD1;
    uint32_t vbo_id : TEXCOORD2;
    float2 texcoord : TEXCOORD3;
    uint32_t primitive_id : SV_PrimitiveID;
    // float4 current_pos : CURRENT_POS;
    // float4 old_pos : OLD_POS;
};

struct FSOutput {
    float4 vbid : SV_Target0;
    float4 vbinst : SV_Target1;
};

// Engel's packing function (without alpha bit)
inline uint32_t pack_vbid(bool opaque, uint32_t draw_id, uint32_t primitive_id) {
    uint32_t drawid_primid = ((draw_id << 23) & 0x7F800000) | (primitive_id & 0x007FFFFF);
    return opaque ? drawid_primid : (1 << 31) | drawid_primid;
}

inline uint32_t pack_moid(uint32_t mat_id, uint32_t obj_id) {
    return (obj_id << 16) | (mat_id & 0xFFFF);
}

static inline float4 unpackUnorm4x8(uint packedValue) {
    const float inv255 = 1.0 / 255.0;

    uint32_t r = packedValue & 0xFF;
    uint32_t g = (packedValue >> 8) & 0xFF;
    uint32_t b = (packedValue >> 16) & 0xFF;
    uint32_t a = (packedValue >> 24) & 0xFF;

    return float4(float(r), float(g), float(b), float(a)) * inv255;
}

[shader("fragment")]
FSOutput main(FSInput input) {
    FSOutput output;

    uint32_t draw_index = input.draw_id + input.instance_id;
    interop::DrawDescription draw_info = egd.draw_descriptions_ptr[draw_index];
    interop::Mesh mesh = g_fd.mesh_ptr[draw_info.mesh_index];
    interop::SubMesh submesh = g_fd.submesh_ptr[mesh.submesh_offset + draw_info.submesh_index];
    interop::Material material = g_fd.material_ptr[submesh.material];

    float opacity = material.base_color_factor.a;
    if (material.base_color.valid()) {
        Texture2D color_texture = g_srv_textures[NonUniformResourceIndex(material.base_color.texture)];
        SamplerState sampler = g_samplers[NonUniformResourceIndex(material.base_color.sampler)];
        opacity *= color_texture.Sample(sampler, input.texcoord).w;
    }

    if (material.alpha_mode == interop::AlphaMode::eMask && opacity < material.alpha_cutoff) {
        discard;
    }
    
    output.vbid = unpackUnorm4x8(pack_vbid(true, input.draw_id, input.instance_id + 1u));
    output.vbinst = unpackUnorm4x8(pack_moid(input.vbo_id, input.instance_id));

    // Calculate motion vectors
    // output.mv = encode_velocity(input.currentPos.xy - input.oldPos.xy);

    return output;
}