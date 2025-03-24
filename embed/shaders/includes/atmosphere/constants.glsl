#ifndef ATMOSPHERE_CONSTANTS_GLSL
#define ATMOSPHERE_CONSTANTS_GLSL

const int integralStepCount = 16;
const int lightIntegralStepCount = 8;

const float atmosphere_planet_radius = 6371e3;
const float atmosphere_radius = 6531e3;
const float max_altitude = atmosphere_radius - atmosphere_planet_radius;
const float maxScatterDistance = max_altitude * 2.0;

const vec3 DefaultColor = vec3(0.01);
const float BaseIntencity = 5.0;

const float RayleighScaleHeight = 8e3;
const vec3 BetaRayleigh = vec3(5.8e-6, 13.5e-6, 33.1e-6);

const float MieScaleHeight = 1.2e3;
const vec3 BetaMie = vec3(21e-6);

const float AbsorptionScaleHeight = 30e3;
const vec3 BetaAbsorption = vec3(2.04e-5, 4.97e-5, 1.95e-6);
const float FalloffAbsorbtion = 4e3;
const vec3 BetaAmbient = vec3(0.01);

#endif // ATMOSPHERE_CONSTANTS_GLSL