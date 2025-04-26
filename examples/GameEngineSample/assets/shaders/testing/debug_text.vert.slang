#include "debug_text.h"

struct VSInput {
    float2 pos : POSITION;
    int instance_id : SV_InstanceID;
    int vertex_id : SV_VertexID;
};

struct VSOutput {
    float2 uv : TEXCOORD0;
    nointerpolation uint word : TEXCOORD1;
    float4 fg_color : COLOR0;
    float4 bg_color : COLOR1;
    float4 position : SV_Position;
};

[[vk::push_constant]] ConstantBuffer<debug_text::PushConstants> pc;

[shader("vertex")]
VSOutput main(VSInput input) {
    debug_text::DebugTextData wd = pc.word_data[input.instance_id];

    float2 scale_factor = float2(1.0, 2.0) * pc.inv_viewport_resolution;

    VSOutput output;
    output.word = wd.word;
    output.fg_color = float4(wd.fg_color) / 255.0;
    output.bg_color = float4(wd.bg_color) / 255.0;
    output.uv = float2((12 >> input.vertex_id) & 1, (9 >> input.vertex_id) & 1);
    output.position = float4(float2(-1.0, -1.0) + (input.pos * wd.scale + wd.pos) * scale_factor, 0.0, 1.0);

    return output;
}