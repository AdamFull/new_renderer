#pragma once

#include "constants.h"

namespace atmosphere {
	// Dimensions
	internal static const float k_planet_radius = 6371e3;
	internal static const float k_atmosphere_height = 100e3;
	internal static const float k_rayleigh_height = 8e3;
	internal static const float k_mie_height = 1.2e3;
	internal static const float k_ozone_peak_level = 30e3;
	internal static const float k_ozone_falloff = 3e3;
	
	// Scattering coefficients
	internal static const float3 k_beta_ray = float3(3.8e-6, 13.5e-6, 33.1e-6); // float3(5.5e-6, 13.0e-6, 22.4e-6)
    internal static const float3 k_beta_mie = float3(21e-6);
    internal static const float3 k_beta_ozone = float3(2.04e-5, 4.97e-5, 1.95e-6);

    // Other
    internal static const float k_sun_illuminance = 128000.0;
    internal static const float k_moon_illuminance = 0.32;
    internal static const float k_space_illuminance = 0.01;

    // Quality settings
    internal static const int k_integral_step_count = 12;
    internal static const int k_light_integral_step_count = 4;

    internal static const float k_atmosphere_radius = k_planet_radius + k_atmosphere_height;
    internal static const float k_max_scatter_distance = k_atmosphere_height * 2.0;

    internal static const float3 k_default_color = float3(0.01);
    internal static const float k_base_intencity = 5.0;

    internal static const float3 k_beta_ambient = float3(0.01);

    inline float2 ray_sphere_intersect(float3 r0, float3 rd, float3 s0, float sr) {
        // - r0: ray origin
        // - rd: normalized ray direction
        // - s0: sphere center
        // - sr: sphere radius
        // - Returns distance from r0 to first intersecion with sphere,
        //   or -1.0 if no intersection.

        float3 oc = r0 - s0;
        float a = dot(rd, rd);
        float half_b = dot(oc, rd);
        float c = dot(oc, oc) - sr * sr;
        float discriminant = half_b * half_b - a * c;

        if (discriminant < 0.0) {
            return float2(1e5, -1e5);
        }

        float sq_d = sqrt(discriminant);

        float t1 = (-half_b - sq_d) / a;
        float t2 = (-half_b + sq_d) / a;
        return float2(t1, t2);
    }

    inline float phase_rayleight(float cos_angle)
    {
        float cos_angle_square = cos_angle * cos_angle;
        return ((3.0 * constants::pi) / 16.0) * (1.0 + cos_angle_square);
    }

    inline float phase_mie(float cos_angle, float g)
    {
        float cos_angle_square = cos_angle * cos_angle;
        float g_square = g * g;
        return ((3.0 * constants::pi) / 8.0) * (1.0 - g_square) * (1.0 + cos_angle_square) / ((2.0 + g_square) * pow(1.0 + g_square - 2. * g * cos_angle, 1.5));
    }

    // Calculate Rayleight, Mie and Absorbtion density
    inline float3 calculate_density(float height, float deltaHeight) {
        const float2 scaleHeight = float2(k_rayleigh_height, k_mie_height);

        // Calculate density for Rayleight and Mie both
        float3 density = float3(exp(-height / scaleHeight), 0.0);

        // Calculate adsorption (Ozone)
        float denom = (k_ozone_peak_level - height) / k_ozone_falloff;
        density.z = (1.0 / (denom * denom + 1.0)) * density.x;

        density *= deltaHeight;

        return density;
    }

    float3 scatter(float3 origin, float3 direction, float3 sunDirection, float sunIntensity) {
        // Trace ray from viewer position to atmosphere
        float2 intersection = ray_sphere_intersect(origin, direction, float3(0.0), k_atmosphere_radius);

        if (intersection.x > intersection.y) {
            return k_default_color;
        }

        float hit_distance = max(intersection.x, intersection.y);
        if (hit_distance < 0.0) {
            return k_default_color;
        }

        hit_distance = min(hit_distance, k_max_scatter_distance);

        // Calculate point where ray hit a sphere
        float3 hit_position = direction * hit_distance;
        float3 ray_step = hit_position / float(k_integral_step_count);
        float step_height = length(ray_step);

        float3 optical_depth = float3(0.0);
        float3 total_rayleight = float3(0.0);
        float3 total_mie = float3(0.0);

        // Solve first integral
        [unroll]
        for (int i = 0; i < k_integral_step_count; ++i) {
            // Calculate current ray step from VIEWER to SKY
            float3 integral_pos = origin + ray_step * float(i);
            float integral_height = length(integral_pos) - k_planet_radius;

            // Accumulate air density
            float3 density = calculate_density(integral_height, step_height);
            optical_depth += density;

            // Trace ray from current integral position to sun direction
            float2 sun_intersection = ray_sphere_intersect(integral_pos, -sunDirection, float3(0.0), k_atmosphere_radius);
            float sun_hit_distance = max(sun_intersection.x, sun_intersection.y);

            // Calculate point where ray hit a sphere
            float3 sun_hit_position = -sunDirection * sun_hit_distance;
            float3 sun_ray_step = sun_hit_position / float(k_light_integral_step_count);
            float sun_step_height = length(sun_ray_step);

            float3 sun_optical_depth = float3(0.0);

            // Solve second integral
            [unroll]
            for (int j = 0; j < k_light_integral_step_count; ++j) {
                // Calculate position from INTEGRAL POSITION to SUN
                float3 sun_integral_pos = integral_pos + sun_ray_step * float(j);
                float sun_integral_height = length(sun_integral_pos) - k_planet_radius;

                // Accumulate sun density
                float3 sun_density = calculate_density(sun_integral_height, sun_step_height);
                sun_optical_depth += sun_density;
            }

            // Calculate light attenuation
            float3 attenuation = exp(-(k_beta_ray * (optical_depth.x + sun_optical_depth.x) + k_beta_mie * (optical_depth.y + sun_optical_depth.y) + k_beta_ozone * (optical_depth.z + sun_optical_depth.z)));

            // Accumulate Rayleight and Mie color
            total_rayleight += density.x * attenuation;
            total_mie += density.y * attenuation;
        }

        // Angle between viewer direction and sun direction
        float angle = dot(normalize(direction), -sunDirection);

        // Calculate total scattered color
        float3 rayleight_color = k_base_intencity * phase_rayleight(angle) * k_beta_ray * total_rayleight;
        float3 mie_color = sunIntensity * phase_mie(angle, 0.45) * k_beta_mie * total_mie;
        float3 sun_color = sunIntensity * phase_mie(angle, 0.999) * k_beta_mie * total_mie;

        return rayleight_color + mie_color + sun_color;
    }
}