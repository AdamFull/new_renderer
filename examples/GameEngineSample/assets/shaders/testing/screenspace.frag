#include "shader_inputs.glsl"
#include "util.glsl"

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

layout(set = 1, binding = 1) uniform sampler2D sample_image0;

void main() 
{
	vec2 texel_size = 1.0 / vec2(textureSize(sample_image0, 0));
	vec4 color = texture(sample_image0, inUV);

	// https://www.shadertoy.com/view/dlcfRX
	//color.rgb = mat3(.842, .0423, .0424, .0784, .878, .0784, .0792, .0792, .879) * color.rgb;
	//color.rgb = clamp((log2(color.rgb) + 12.47393) / 16.5, vec3(0), vec3(1));
	//color.rgb = .5 + .5 * sin(((-3.11 * color.rgb + 6.42) * color.rgb - .378) * color.rgb - 1.44);
	//color.rgb = mat3(1.2, -.053, -.053, -.1, 1.15, -.1, -.1, -.1, 1.15) * color.rgb;

	outFragcolor = color;
}