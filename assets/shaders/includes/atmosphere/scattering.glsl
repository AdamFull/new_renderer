#ifndef ATMOSPHERE_SCATTERING_GLSL
#define ATMOSPHERE_SCATTERING_GLSL

#include "math.glsl"
#include "atmosphere/constants.glsl"

#extension GL_EXT_control_flow_attributes : enable

vec2 raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr) 
{
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.

    vec3 oc = r0 - s0;
    float a = dot(rd, rd);
    float half_b = dot(oc, rd);
    float c = dot(oc, oc) - sr * sr;
    float discriminant = half_b * half_b - a * c;

    if (discriminant < 0.0)
        return vec2(1e5, -1e5);

    float sq_d = sqrt(discriminant);

    float t1 = (-half_b - sq_d) / a;
    float t2 = (-half_b + sq_d) / a;
    return vec2(t1, t2);
}

float phaseRayleight(float cosAngle)
{
    float cosAngleSquare = cosAngle * cosAngle;
    return ((3.0 * PI) / 16.0) * (1.0 + cosAngleSquare);
}

float phaseMie(float cosAngle, float g)
{
    float cosAngleSquare = cosAngle * cosAngle;
    float gSquare = g * g;
    return ((3.0 * PI) / 8.0) * (1.0 - gSquare) * (1.0 + cosAngleSquare) / ((2.0 + gSquare) * pow(1.0 + gSquare - 2. * g * cosAngle, 1.5));
}

// Calculate Rayleight, Mie and Absorbtion density 
vec3 calculateDensity(float height, float deltaHeight)
{
    const vec2 scaleHeight = vec2(RAYLEIGH_HEIGHT, MIE_HEIGHT);

    // Calculate density for Rayleight and Mie both
    vec3 density = vec3(exp(-height / scaleHeight), 0.0);

    // Calculate adsorption (Ozone)
    float denom = (OZONE_PEAK_LEVEL - height) / OZONE_FALLOFF;
    density.z = (1.0 / (denom * denom + 1.0)) * density.x;

    density *= deltaHeight;

    return density;
}

vec3 atmosphere(vec3 origin, vec3 direction, vec3 sunDirection, float sunIntensity)
{
    // Trace ray from viewer position to atmosphere
    vec2 intersection = raySphereIntersect(origin, direction, vec3(0.0), ATMOSPHERE_RADIUS);

    if (intersection.x > intersection.y)
        return DefaultColor;

    float hitDistance = max(intersection.x, intersection.y);

    if (hitDistance < 0.0)
        return DefaultColor;

    hitDistance = min(hitDistance, maxScatterDistance);

    // Calculate point where ray hit a sphere
    vec3 hitPosition = direction * hitDistance;
    vec3 rayStep = hitPosition / float(INTEGRAL_STEP_COUNT);
    float stepHeight = length(rayStep);

    vec3 opticalDepth = vec3(0.0);
    vec3 totalRayleight = vec3(0.0);
    vec3 totalMie = vec3(0.0);

    // Solve first integral
    [[unroll]]
    for (int i = 0; i < INTEGRAL_STEP_COUNT; ++i)
    {
        // Calculate current ray step from VIEWER to SKY
        vec3 integralPos = origin + rayStep * float(i);
        float integralHeight = length(integralPos) - PLANET_RADIUS;

        // Accumulate air density
        vec3 density = calculateDensity(integralHeight, stepHeight);
        opticalDepth += density;

        // Trace ray from current integral position to sun direction
        vec2 sunIntersection = raySphereIntersect(integralPos, -sunDirection, vec3(0.0), ATMOSPHERE_RADIUS);
        float sunHitDistance = max(sunIntersection.x, sunIntersection.y);

        // Calculate point where ray hit a sphere
        vec3 sunHitPosition = -sunDirection * sunHitDistance;
        vec3 sunRayStep = sunHitPosition / float(LIGHT_INTEGRAL_STEP_COUNT);
        float sunStepHeight = length(sunRayStep);

        vec3 sunOpticalDepth = vec3(0.0);

        // Solve second integral
        [[unroll]]
        for (int j = 0; j < LIGHT_INTEGRAL_STEP_COUNT; ++j)
        {
            // Calculate position from INTEGRAL POSITION to SUN
            vec3 sunIntegralPos = integralPos + sunRayStep * float(j);
            float sunIntegralHeight = length(sunIntegralPos) - PLANET_RADIUS;

            // Accumulate sun density
            vec3 sunDensity = calculateDensity(sunIntegralHeight, sunStepHeight);
            sunOpticalDepth += sunDensity;
        }

        // Calculate light attenuation
        vec3 attenuation = exp(-(BETA_RAY * (opticalDepth.x + sunOpticalDepth.x) + BETA_MIE * (opticalDepth.y + sunOpticalDepth.y) + BETA_OZONE * (opticalDepth.z + sunOpticalDepth.z)));

        // Accumulate Rayleight and Mie color
        totalRayleight += density.x * attenuation;
        totalMie += density.y * attenuation;
    }

    // Angle between viewer direction and sun direction
    float angle = dot(normalize(direction), -sunDirection);

    // Calculate total scattered color
    vec3 rayleightColor = BaseIntencity * phaseRayleight(angle) * BETA_RAY * totalRayleight;
    vec3 mieColor = sunIntensity * phaseMie(angle, 0.45) * BETA_MIE * totalMie;
    vec3 sunColor = sunIntensity * phaseMie(angle, 0.999) * BETA_MIE * totalMie;

    return rayleightColor + mieColor + sunColor;
}

#endif // ATMOSPHERE_SCATTERING_GLSL