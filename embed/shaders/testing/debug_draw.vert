#include "shader_inputs.glsl"

layout (location = 0) in vec3 inPos;

layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

layout (push_constant) uniform PushConstants 
{
	vec2 scale;
	vec2 translate;
} constants;

out gl_PerVertex 
{
	vec4 gl_Position;   
};

void main() 
{
	outUV = inUV;
	outColor = inColor;

	#ifdef DEBUG_3D
		FCamera camera = GetCamera(frameData.defaultCameraID);
		mat4 viewProjection = GetViewProjectionMatrix(camera.matrices_offset);

		gl_Position = viewProjection * vec4(inPos, 1.0);
	#else
		gl_Position = vec4(inPos.xy * constants.scale + constants.translate, 0.0, 1.0);
	#endif
}