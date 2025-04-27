Texture2D<float4> texture0;
SamplerState sampler0;

struct PSInput {
    float2 tex_coord : TEXCOORD0;
};

struct PSOutput {
    float4 out_color : SV_TARGET0;
};

[shader("fragment")]
PSOutput main(PSInput input) {
    PSOutput output;

    uint width, height;
    texture0.GetDimensions(width, height);
    float2 texel_size = 1.0 / float2(width, height);
    output.out_color = texture0.Sample(sampler0, input.tex_coord);

    return output;
}