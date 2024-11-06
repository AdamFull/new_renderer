layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

layout(set = 1, binding = 1) uniform sampler2D copy_target;

void main() 
{
	outFragcolor = texture(copy_target, inUV);
}