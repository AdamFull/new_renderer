#include "imgui.h"

struct VSInput {
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VSOutput {
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float4 position : SV_Position;
};

[[vk::push_constant]] ConstantBuffer<imgui::PushConstants> pc;

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput output;

    output.uv = input.uv;
    output.color = input.color;

    output.position = float4(input.pos * pc.scale + pc.translate, 0.0, 1.0);

    return output;
}