#ifndef SHADOW_SAMPLING_GLSL
#define SHADOW_SAMPLING_GLSL

#include "util.glsl"

const bool enableCascadedPCF = true;

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

const vec2 diskOffsets[16] = {
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790)
};

float cascadeShadowProjection(sampler2DArrayShadow shadowmap_tex, vec4 shadowCoord, vec2 offset, uint cascadeIndex, float bias)
{
	return texture(shadowmap_tex, vec4(shadowCoord.st + offset, cascadeIndex, shadowCoord.z - bias));
}

float cascadeShadowFilterPCF(sampler2DArrayShadow shadowmap_tex, vec4 sc, uint cascadeIndex, float bias)
{
	ivec2 texDim = textureSize(shadowmap_tex, 0).xy;
	vec2 texelSize = 1.0 / vec2(texDim);

	float shadowFactor = 0.0;

	// Random rotation and jitter
    const float rotationAngle = random(sc.xy) * 6.2831853;
    const float sinAngle = sin(rotationAngle);
    const float cosAngle = cos(rotationAngle);

	const float jitter = random(sc.xy + vec2(1.0, 1.0)) * 0.5; // Random jitter scale (0 to 0.5)

	[[unroll]]
	for (int i = 0; i < 16; ++i)
    {
        // Rotate the disk pattern
        vec2 rotatedOffset = vec2(
            diskOffsets[i].x * cosAngle - diskOffsets[i].y * sinAngle,
            diskOffsets[i].x * sinAngle + diskOffsets[i].y * cosAngle
        );

        // Apply jitter to the offset
        rotatedOffset += random(sc.xy + vec2(i, i)) * jitter;

        // Scale by texel size
        shadowFactor += cascadeShadowProjection(shadowmap_tex, sc, rotatedOffset * texelSize * 1.5, cascadeIndex, bias);
    }
	return shadowFactor * 0.0625;
}

// For spotlight source
float directionalShadowProjection(sampler2DArrayShadow shadowmap_tex, vec4 P, vec2 offset, float bias, int layer)
{
	vec4 shadowCoord = P / P.w;
	shadowCoord.st = shadowCoord.st * 0.5 + 0.5;
	return texture(shadowmap_tex, vec4(shadowCoord.st + offset, layer, shadowCoord.z - bias));
}

float directionalShadowFilterPCF(sampler2DArrayShadow shadowmap_tex, vec4 sc, float bias, int layer)
{
	ivec2 texDim = textureSize(shadowmap_tex, 0).xy;
	vec2 texelSize = 1.0 / vec2(texDim);

	float shadowFactor = 0.0;
	
	// Random rotation and jitter
    const float rotationAngle = random(sc.xy) * 6.2831853;
    const float sinAngle = sin(rotationAngle);
    const float cosAngle = cos(rotationAngle);

	const float jitter = random(sc.xy + vec2(1.0, 1.0)) * 0.5; // Random jitter scale (0 to 0.5)

	[[unroll]]
	for (int i = 0; i < 16; ++i)
    {
        // Rotate the disk pattern
        vec2 rotatedOffset = vec2(
            diskOffsets[i].x * cosAngle - diskOffsets[i].y * sinAngle,
            diskOffsets[i].x * sinAngle + diskOffsets[i].y * cosAngle
        );

        // Apply jitter to the offset
        rotatedOffset += random(sc.xy + vec2(i, i)) * jitter;
		shadowFactor += directionalShadowProjection(shadowmap_tex, sc, rotatedOffset * texelSize * 1.5, bias, layer);
	}
	return shadowFactor *  0.0625;
}

// For omni directional source
//Based on https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float omniShadowProjection(samplerCubeArrayShadow shadowmap_tex, vec3 shadowClip, vec3 offset, float bias, float radius, int layer)
{
	float currentDist = length(shadowClip) / radius;
	return texture(shadowmap_tex, vec4(shadowClip + offset, layer), currentDist - bias);
}

float omniShadowFilterPCF(samplerCubeArrayShadow shadowmap_tex, vec3 shadowClip, float bias, float radius, int layer)
{
	float shadowFactor = 0.0;
	float samples = 4.0;
	const float offset  = 0.1;

	[[unroll]]
	for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	{
		[[unroll]]
		for(float y = -offset; y < offset; y += offset / (samples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (samples * 0.5))
				shadowFactor += omniShadowProjection(shadowmap_tex, shadowClip, vec3(x, y, z), bias, radius, layer);
		}
	}
	shadowFactor /= (samples * samples * samples);
	return shadowFactor;
}

#endif // SHADOW_SAMPLING_GLSL