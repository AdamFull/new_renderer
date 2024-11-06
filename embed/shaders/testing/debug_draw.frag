//#include "shader_inputs"

layout (set = 1, binding = 1) uniform sampler2D font_sampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
	vec4 readed_color = texture(font_sampler, inUV);
	if(readed_color.a < 0.5)
		discard;

	outColor = inColor * readed_color;
}