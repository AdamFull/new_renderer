//#include "shader_inputs"

layout (binding = 0) uniform texture2D texture0;
layout (binding = 1) uniform sampler sampler0;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = inColor * texture(sampler2D(texture0, sampler0), inUV);
}