#include "shader_inputs"

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inPos;
layout(location = 2) flat in uint materialID;

layout(push_constant) uniform ShadowmapData
{
	uint32_t callID;
	int32_t shadowIndex;
	int32_t layer;
} shadowData;

void main() 
{

	if(materialID != ~0u)
	{
		FMaterial mat = GetMaterial(materialID);
		if(mat.baseColorTexture != ~0u)
		{
			float smaple = sampleTexture2D(mat.baseColorTexture, 0, inUV).a;
			if(smaple < 1.f)
				discard;
		}
	}

#if defined(OMNI_SHADOW)
	FOmniShadowCommit shadow = getOmniShadowCommit(shadowData.shadowIndex);
	FLightCommit light = getLight(shadow.index);
	
	// get distance between fragment and light source
	float lightDistance = length(inPos.xyz - light.position);
	
	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / shadow.farPlane;
	
	// write this as modified depth
	gl_FragDepth = lightDistance;
#endif
}