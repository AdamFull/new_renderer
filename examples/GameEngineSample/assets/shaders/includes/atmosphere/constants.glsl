#ifndef ATMOSPHERE_CONSTANTS_GLSL
#define ATMOSPHERE_CONSTANTS_GLSL

// Dimensions
#define PLANET_RADIUS 6371e3
#define ATMOSPHERE_HEIGHT 100e3
#define RAYLEIGH_HEIGHT 8e3
#define MIE_HEIGHT 1.2e3
#define OZONE_PEAK_LEVEL 30e3
#define OZONE_FALLOFF 3e3
// Scattering coefficients
#define BETA_RAY vec3(3.8e-6, 13.5e-6, 33.1e-6) // vec3(5.5e-6, 13.0e-6, 22.4e-6)
#define BETA_MIE vec3(21e-6)
#define BETA_OZONE vec3(2.04e-5, 4.97e-5, 1.95e-6)
// Other
#define SUN_ILLUMINANCE 128000.0
#define MOON_ILLUMINANCE 0.32
#define SPACE_ILLUMINANCE 0.01
// Quality settings
#define INTEGRAL_STEP_COUNT 12
#define LIGHT_INTEGRAL_STEP_COUNT 4

const float ATMOSPHERE_RADIUS = PLANET_RADIUS + ATMOSPHERE_HEIGHT;

const float maxScatterDistance = ATMOSPHERE_HEIGHT * 2.0;

const vec3 DefaultColor = vec3(0.01);
const float BaseIntencity = 5.0;

const vec3 BetaAmbient = vec3(0.01);

#endif // ATMOSPHERE_CONSTANTS_GLSL