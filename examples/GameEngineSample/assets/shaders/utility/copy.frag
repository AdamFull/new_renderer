layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

layout(set = 1, binding = 1) uniform texture2D texture0;
layout(set = 1, binding = 2) uniform sampler sampler0;

void main() 
{
	outFragcolor = texture(sampler2D(texture0, sampler0), inUV);
}