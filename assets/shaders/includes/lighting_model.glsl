#ifndef LIGHTING_MODEL_GLSL
#define LIGHTING_MODEL_GLSL

#include "math.glsl"
#include "util.glsl"

// Sampled material data
struct MaterialSample 
{
	vec3 albedo;
	float opacity;

	vec3 emission;

	float metallic;
	float roughness;

	vec3 normal;

	float ao;

	float ior;
};

vec3 get_cube_side_direction(in vec2 uv, in uint side) {
	const vec2 tex = uv * 2.0f - 1.0f;
	if(side == 0) return vec3(1.0f, -tex.y, -tex.x); // Front
	if(side == 1) return vec3(-1.0f, -tex.y, tex.x); // Back
	if(side == 2) return vec3(tex.x, 1.0f, tex.y); // Right
	if(side == 3) return vec3(tex.x, -1.0f, -tex.y); // Left
	if(side == 4) return vec3(tex.x, -tex.y, 1.0f); // Top
	if(side == 5) return vec3(-tex.x, -tex.y, -1.0f); // Bottom
	return vec3(1.0);
}

// Normal Distribution function --------------------------------------
float D_GGX(in float dotNH, in float roughness) {
	const float alpha = roughness * roughness;
	const float alpha2 = alpha * alpha;
	const float denom = dotNH * dotNH * (alpha2 - 1.0f) + 1.0f;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(in float dotNL, in float dotNV, in float roughness) {
	const float r = (roughness + 1.0f);
	const float k = (r*r) / 8.0f;
	const float GL = dotNL / (dotNL * (1.0f - k) + k);
	const float GV = dotNV / (dotNV * (1.0f - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F0FromIOR(in float ior) {
	const float a = (1.f - ior) / (1.f + ior);
	return vec3(a * a);
}

vec3 F_Schlick(in float cosTheta, in float f90, in vec3 F0) {
	return F0 + (f90 - F0) * pow(1.0f - cosTheta, 5.0f);
}

vec3 F_SchlickR(in float cosTheta, vec3 F0, in float roughness) {
	return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

vec2 hammersley2d(in uint i, in uint N) {
	// Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	uint bits = (i << 16u) | (i >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	const float rdi = float(bits) * 2.3283064365386963e-10;
	return vec2(float(i) /float(N), rdi);
}

vec3 hemispherepoint_uniform(in float u, in float v) {
	const float phi = v * TWO_PI;
	const float cosTheta = 1.0 - u;
	const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
vec3 importanceSample_GGX(in vec2 Xi, in float roughness, in vec3 normal) {
	// Maps a 2D point to a hemisphere with spread based on roughness
	const float alpha = roughness * roughness;
	const float phi = TWO_PI * Xi.x + random(normal.xz) * 0.1f;
	const float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (alpha*alpha - 1.0f) * Xi.y));
	const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	const vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	// Tangent space
	const vec3 up = abs(normal.z) < 0.999 ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
	const vec3 tangentX = normalize(cross(up, normal));
	const vec3 tangentY = normalize(cross(normal, tangentX));

	// Convert to world Space
	return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

vec3 prefilteredReflection(in vec3 R, in float roughness, in samplerCube prefiltered) {
	const float lod = roughness * float(textureQueryLevels(prefiltered));
	const float lodf = floor(lod);
	const float lodc = ceil(lod);
	const vec3 a = pow(textureLod(prefiltered, R, lodf).rgb, vec3(2.2f));
	const vec3 b = pow(textureLod(prefiltered, R, lodc).rgb, vec3(2.2f));
	return mix(a, b, lod - lodf);
}


// microfacet model developed by Torrance-Sparrow
vec3 evaluateCookTorrenceSpecularBRDF(in float D, in float G, in vec3 F, in float cosThetaI, in float cosThetaO) {
    return vec3((D * G * F) / (4.f * cosThetaI * cosThetaO));

}

vec3 specularContribution(in vec3 diffuse, in vec3 L, in vec3 V, in vec3 N, in vec3 F0, in float metallic, in float roughness) {
	// Precalculate vectors and dot products	
	const vec3 H = normalize (V + L);
	const float dotNH = clamp(dot(N, H), 0.0, 1.0);
	const float dotNV = clamp(dot(N, V), 0.0, 1.0);
	const float dotNL = clamp(dot(N, L), 0.0, 1.0);

	vec3 color = vec3(0.0f);

	if (dotNL > 0.0f) 
	{
		// D = Normal distribution (Distribution of the microfacets)
		const float D = D_GGX(dotNH, roughness);
		// G = Geometric shadowing term (Microfacets shadowing)
		const float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		const vec3 F = F_Schlick(dotNV, 1.0, F0);
		const vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
		const vec3 kD = (vec3(1.0f) - F) * (1.0f - metallic);
		color += (kD * diffuse / PI + spec) * dotNL;
	}

	return color;
}

#endif // LIGHTING_MODEL_GLSL