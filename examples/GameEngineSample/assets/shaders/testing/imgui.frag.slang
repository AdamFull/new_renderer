
[[vk::binding(0, 0)]]
Texture2D<float4> texture0 : register(t0, space0);
[[vk::binding(0, 1)]]
SamplerState sampler0 : register(s1, space0);

struct PSInput {
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct PSOutput {
    float4 color : SV_Target0;
};

[shader("fragment")]
PSOutput main(PSInput input) {
    PSOutput output;
    output.color = input.color * texture0.Sample(sampler0, input.uv);
    return output;
}