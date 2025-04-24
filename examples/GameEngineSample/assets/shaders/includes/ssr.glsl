#ifndef SSR_GLSL
#define SSR_GLSL

#include "util.glsl"

const float rayStep = 0.2f;
const int iterationCount = 100;
const float distanceBias = 0.05f;
const int sampleCount = 4;
const float samplingCoefficient = 0.01f;
const float reflectionSpecularFalloffExponent = 3.f;
const int debugDraw = -1;
const int isBinarySearchEnabled = 1;
const int isAdaptiveStepEnabled = 1;
const int isExponentialStepEnabled = -1;
const int isSamplingEnabled = 1;

vec3 generatePositionFromDepth(vec2 texturePos, float depth) 
{
	Camera camera = GetCamera(frameData.defaultCameraID);

	vec4 ndc = vec4((texturePos - 0.5) * 2, depth, 1.f);
	vec4 inversed = GetInvProjectionMatrix(camera.matrices_offset) * ndc;
	inversed /= inversed.w;
	return inversed.xyz;
}

vec2 generateProjectedPosition(vec3 pos)
{
	Camera camera = GetCamera(frameData.defaultCameraID);

	vec4 samplePosition = GetProjectionMatrix(camera.matrices_offset) * vec4(pos, 1.f);
	samplePosition.xy = (samplePosition.xy / samplePosition.w) * 0.5 + 0.5;
	return samplePosition.xy;
}

vec2 SSRRaycast(in vec3 position, in vec3 reflection, in sampler2D depth_sampler, out vec3 color) 
{
	vec3 stp = rayStep * reflection;
	vec3 marchingPosition = position + stp;
	float delta;
	float depthFromScreen;
	vec2 screenPosition;
	
	int i = 0;
	for (; i < iterationCount; i++) 
	{
		screenPosition = generateProjectedPosition(marchingPosition);
		depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(depth_sampler, screenPosition).x).z);
		delta = abs(marchingPosition.z) - depthFromScreen;

		if (abs(delta) < distanceBias) 
		{
			color = vec3(1);
			if(debugDraw > 0)
				color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
			return screenPosition;
		}

		if (isBinarySearchEnabled > 0 && delta >= 0)
			break;

		if (isAdaptiveStepEnabled > 0)
		{
			float directionSign = sign(abs(marchingPosition.z) - depthFromScreen);
			//this is sort of adapting step, should prevent lining reflection by doing sort of iterative converging
			//some implementation doing it by binary search, but I found this idea more cheaty and way easier to implement
			stp = stp * (1.0 - rayStep * max(directionSign, 0.0));
			marchingPosition += stp * (-directionSign);
		}
		else
			marchingPosition += stp;
	
		if (isExponentialStepEnabled > 0)
			stp *= 1.05;
    }


	if(isBinarySearchEnabled > 0)
	{
		for(; i < iterationCount; i++){
			
			stp *= 0.5;
			marchingPosition = marchingPosition - stp * sign(delta);
			
			screenPosition = generateProjectedPosition(marchingPosition);
			depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(depth_sampler, screenPosition).x).z);
			delta = abs(marchingPosition.z) - depthFromScreen;
			
			if (abs(delta) < distanceBias) 
			{
                color = vec3(1);
                if(debugDraw > 0)
                    color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
				return screenPosition;
			}
		}
	}
	
    return vec2(0.0);
}

vec3 computeSSR(in vec2 uv, in vec3 position, in vec3 normal, in float metallic, in float roughness, in sampler2D depth_sampler, in sampler2D color_sampler)
{
	vec3 ssr = vec3(0.0);

	const float roughnessThreshold = 0.7f;
	bool ssrEnabled = roughness < roughnessThreshold;
	if(ssrEnabled)
	{
		vec3 reflectionDirection = normalize(reflect(position, normalize(normal)));
		vec3 firstBasis = normalize(cross(vec3(0.f, 0.f, 1.f), reflectionDirection));
		vec3 secondBasis = normalize(cross(reflectionDirection, firstBasis));
		vec4 resultingColor = vec4(0.f);

		for (int i = 0; i < sampleCount; i++) 
		{
			vec2 coeffs = vec2(random(uv + vec2(0, i)) + random(uv + vec2(i, 0))) * samplingCoefficient;
			vec3 reflectionDirectionRandomized = reflectionDirection + firstBasis * coeffs.x + secondBasis * coeffs.y;

			vec3 debugColor;
			vec2 coords = SSRRaycast(position, normalize(reflectionDirectionRandomized), depth_sampler, debugColor);
			if (coords != vec2(0.0f))
			{
				vec4 reflColor = textureLod(color_sampler, coords, 0);
				reflColor.rgb *= debugColor;

				vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
				float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
				float reflectionMultiplier = pow(metallic, reflectionSpecularFalloffExponent) * screenEdgefactor * -reflectionDirection.z;

				reflColor.rgb *= clamp(reflectionMultiplier, 0.0, 0.9);

				resultingColor += reflColor;
			}
		}

		if (resultingColor.w != 0)
		{
			resultingColor /= resultingColor.w;
			ssr = resultingColor.rgb;
		}

		//float roughnessMask = saturate(1.0f - (roughness / roughnessThreshold));
		//float ssrWeight = (resultingColor.w * roughnessMask);
		//ssr = clamp(resultingColor.xyz * ssrWeight, 0.0, 1.0);
	}

	return ssr;
}

// Convert view space position to screen UVs (0, 1). Non-linear Z
vec3 ViewPositionToUV(vec3 view, mat4 projection)
{
	vec4 proj = projection * vec4(view, 1.0);
	proj.xyz /= proj.w;
	proj.x = (proj.x + 1.0) / 2.0;
	proj.y = (proj.y + 1.0) / 2.0;
	//proj.y = 1.0 - (proj.y + 1.0) / 2.0;
	return proj.xyz;
}

vec3 ScreenSpaceReflections(in vec3 worldPosition, in vec3 N, in vec3 V, in float R, in sampler2D depthTexture, in sampler2D previousSceneColor, float dither, inout float ssrWeight)
{
	Camera camera = GetCamera(frameData.defaultCameraID);
	mat4 view = GetViewMatrix(camera.matrices_offset);

	vec3 ssr = vec3(0.0);
	vec3 debug = vec3(0.0);
	const float roughnessThreshold = 0.75f;
	bool ssrEnabled = R < roughnessThreshold;
	if(ssrEnabled)
	{
		float reflectionThreshold = 0.0f;
		vec3 reflectionWs = normalize(reflect(-V, N));
		if (dot(V, reflectionWs) <= reflectionThreshold)
		{
			float jitter = dither - 1.0f;
			uint maxSteps = 8;
		
			vec3 rayStartVS = vec3(view * vec4(worldPosition, 1.0));
			float linearDepth = rayStartVS.z;
			vec3 reflectionVs = vec3(view * vec4(reflectionWs, 1.0));
			vec3 rayEndVS = rayStartVS + (reflectionVs * linearDepth);
		
			mat4 projection = GetProjectionMatrix(camera.matrices_offset);
			vec3 rayStart = ViewPositionToUV(rayStartVS, projection);
			vec3 rayEnd = ViewPositionToUV(rayEndVS, projection);
		
			vec3 rayStep = ((rayEnd - rayStart) / float(maxSteps));
			rayStep = rayStep / length(rayEnd.xy - rayStart.xy);
			vec3 rayPos = rayStart + (rayStep * jitter);
			float zThickness = abs(rayStep.z);
		
			uint hitIndex = 0;
			vec3 bestHit = rayPos;
			float prevSceneZ = rayStart.z;
			for (uint currStep = 0; currStep < maxSteps; currStep += 4)
			{
				uvec4 step = uvec4(1u, 2u, 3u, 4u) + currStep;
				vec4 sceneZ = vec4(
					textureLod(depthTexture, rayPos.xy + rayStep.xy * step.x, 0).x,
					textureLod(depthTexture, rayPos.xy + rayStep.xy * step.y, 0).x,
					textureLod(depthTexture, rayPos.xy + rayStep.xy * step.z, 0).x,
					textureLod(depthTexture, rayPos.xy + rayStep.xy * step.w, 0).x
				);
				
				vec4 currentPosition = rayPos.z + rayStep.z * step;
				uvec4 zTest = uvec4(lessThan(abs(sceneZ - currentPosition - zThickness), vec4(zThickness)));
				uint zMask = (((zTest.x << 0u) | (zTest.y << 1u)) | (zTest.z << 2u)) | (zTest.w << 3u);
				
				if(zMask > 0)
				{
					uint firstHit = findLSB(zMask);
					if(firstHit > 0)
						prevSceneZ = sceneZ[firstHit - 1];
		
					bestHit = rayPos + (rayStep * float(currStep + firstHit + 1));
					float zAfter = sceneZ[firstHit] - bestHit.z;
					float zBefore = (prevSceneZ - bestHit.z) + rayStep.z;
					float weight = saturate(zAfter / (zAfter - zBefore));
					vec3 prevRayPos = bestHit - rayStep;
					bestHit = (prevRayPos * weight) + (bestHit * (1.0f - weight));
					hitIndex = currStep + firstHit;
					break;
				}
				prevSceneZ = sceneZ.w;
			}
		
			vec4 hitColor = vec4(0.0);
			if (hitIndex > 0)
			{
				mat4 reprojection = GetReprojectionMatrix(camera.matrices_offset);

				debug += vec3(0.0, 0.5, 0.0);
				vec4 uv = vec4(bestHit.xy, 0.0, 1.0);
				uv = reprojection * uv;
		
				vec2 distanceFromCenter = (vec2(uv.x, uv.y) * 2.0f) - vec2(1.0f, 1.0f);
				float edgeAttenuation = clamp((1.0 - (float(hitIndex / maxSteps))) * 4.0f, 0.0, 1.0);
				edgeAttenuation *= smoothstep(0.0f, 0.5f, clamp(1.0 - dot(distanceFromCenter, distanceFromCenter), 0.0, 1.0));
				vec3 reflectionResult = textureLod(previousSceneColor, uv.xy, 0).rgb;
				hitColor = vec4(reflectionResult, edgeAttenuation);
			}
			float roughnessMask = saturate(1.0f - (R / roughnessThreshold));
			ssrWeight = (hitColor.w * roughnessMask);
			ssr = clamp(hitColor.xyz, 0.0, 1.0);
		}
	}
	return ssr;
	//return debug;
}

float screen_fade(vec2 uv)
{
	vec2 fade = max(12.0f * abs(uv - 0.5f) - 5.0f, 0.0f);
	return saturate(1.0 - dot(fade, fade));
}

bool is_saturated(vec2 uv) {
	return all(greaterThanEqual(uv, vec2(0.0))) &&
		all(lessThanEqual(uv, vec2(1.0)));
}

#if 1
// Settings
const uint  g_sss_max_steps = 32;     // Max ray steps, affects quality and performance.
const float g_sss_ray_max_distance = 0.05f;  // Max shadow length, longer shadows are less accurate.
const float g_sss_thickness = 0.5f;  // Depth testing thickness.
const float g_sss_step_length = g_sss_ray_max_distance / float(g_sss_max_steps);

float ScreenSpaceShadows(vec3 worldPosition, vec3 lightDirection, in sampler2D depthTexture, int stepCount, float rayLength, float ditherOffset)
{
	Camera camera = GetCamera(frameData.defaultCameraID);
	mat4 projection = GetProjectionMatrix(camera.matrices_offset);
	mat4 view = GetViewMatrix(camera.matrices_offset);

	// Compute ray position and direction (in view-space)
	vec3 vs_ray_pos = vec3(view * vec4(worldPosition, 1.0f));
	vec3 ray_dir = vec3(view * vec4(-lightDirection, 0.0f));

	// Compute ray step
	vec3 ray_step = ray_dir * g_sss_ray_max_distance;

	// Ray march towards the light
	float occlusion = 0.0;
	vec2 ray_uv = vec2(0.0f);
	for (uint i = 0; i < g_sss_max_steps; i++) {
		// Step the ray
		vs_ray_pos += ray_step;

		vec4 new_tex_coord = projection * vec4(vs_ray_pos, 1.0);
		new_tex_coord.xyz /= new_tex_coord.w;
		ray_uv = clip_to_uv(new_tex_coord.xy);
		ray_uv = clip_to_uv(vs_ray_pos.xy);

		// Ensure the UV coordinates are inside the screen
		if (is_saturated(ray_uv)) {
			// Compute the difference between the ray's and the camera's depth
			float non_linz = textureLod(depthTexture, ray_uv, 0.0).r;
			float depth_z = linearize_depth(non_linz, camera.z_near, camera.z_far);
			float depth_delta = -vs_ray_pos.z - depth_z;

			// Check if the camera can't "see" the ray (ray depth must be larger than the camera depth, so positive depth_delta)
			if ((depth_delta > 0.01f) && (abs(depth_delta) < g_sss_thickness)) {
				// Mark as occluded
				occlusion = 1.0f;

				// Fade out as we approach the edges of the screen
				occlusion *= screen_fade(ray_uv);

				break;
			}
		}
	}

	return occlusion;
}
#else
float ScreenSpaceShadows(vec3 worldPosition, vec3 lightDirection, in sampler2D depthTexture, int stepCount, float rayLength, float ditherOffset)
{
	Camera camera = GetCamera(frameData.defaultCameraID);
	mat4 projection = GetProjectionMatrix(camera.matrices_offset);
	mat4 view = GetViewMatrix(camera.matrices_offset);

	vec3 vs_ray_pos = vec3(view * vec4(worldPosition, 1.0)).xyz;
	vec3 vs_ray_dir = vec3(view * vec4(-lightDirection, 0.0)).xyz;

	vec3 vs_ray_step = vs_ray_dir * rayLength;

	float occlusion = 0.0;
	vec2 ray_uv = vec2(0.0);
	uint hitCount = 0u;

	const float thickness = 0.5;
	const int max_hits = 3;

	// Raymarch loop: step along the ray and sample the depth texture.
	for (uint i = 0u; i < stepCount; ++i) {
		vs_ray_pos += vs_ray_step;
		ray_uv = view_space_to_uv(vs_ray_pos, projection);

		if (is_saturated(ray_uv)) {
			// Sample the depth texture using textureLod at LOD 0.
			float nonLinz = textureLod(depthTexture, ray_uv, 0.0).r;
			float depth_z = linearize_depth(nonLinz, camera.z_near, camera.z_far);

			// Compute the difference between the current ray depth and the stored depth.
			float depth_delta = -vs_ray_pos.z - depth_z;

			// Check for a hit based on the depth delta conditions.
			if ((depth_delta > 0.1) && (abs(depth_delta) < thickness)) {
				hitCount++;
				if (hitCount >= max_hits) {
					occlusion = 1.0;
					break;
				}
			}
		}
	}

	return occlusion;
}
#endif


#endif // SSR_GLSL