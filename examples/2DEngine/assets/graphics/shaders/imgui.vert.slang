#include "imgui.h"

struct VSOutput {
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float4 position : SV_Position;
};

[[vk::push_constant]] ConstantBuffer<e2d::imgui::PushConstant> pc;

[shader("vertex")]
VSOutput main(uint32_t vertex_index: SV_VertexID) {
    e2d::imgui::Vertex vert = pc.vertices[vertex_index];

    VSOutput output;
    output.uv = vert.uv;
    output.color = interop::decode_color(vert.col);
    output.position = float4(vert.pos * pc.scale + pc.translate, 0.0, 1.0);

    return output;
}