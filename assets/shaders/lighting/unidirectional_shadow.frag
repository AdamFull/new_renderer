#include "shader_inputs.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inPos;
layout(location = 2) flat in uint material;

layout(push_constant) uniform ShadowmapData
{
	uint32_t call_id;
	int32_t shadow_index;
	int32_t layer;
} shadow_data;

void main() 
{

	if(material != ~0u)
	{
		Material mat = GetMaterial(material);
		if(mat.baseColorTexture != ~0u)
		{
			float smaple = SampleTexture2D(mat.baseColorTexture, 0, inUV).a;
			if(smaple < 1.f)
				discard;
		}
	}

#if defined(OMNI_SHADOW)
	OmniShadowCommit shadow = GetOmniShadowCommit(shadow_data.shadow_index);
	LightCommit light = getLight(shadow.index);
	
	// get distance between fragment and light source
	float lightDistance = length(inPos.xyz - light.position);
	
	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / shadow.farPlane;
	
	// write this as modified depth
	gl_FragDepth = lightDistance;
#endif
}