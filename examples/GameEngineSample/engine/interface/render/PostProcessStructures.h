#pragma once

#include <array>
#include "../../ui/Attributes.h"

namespace engine::render {
	const std::array<float, 5> fxaa_quality_variant{ 1.0f, 0.75f, 0.5f, 0.25f, 0.0f };
	const std::array<float, 5> fxaa_threshold_variant{ 0.333f, 0.250f, 0.166f, 0.125f, 0.063f };
	const std::array<float, 3> fxaa_threshold_min_variant{ 0.0833f, 0.0625f, 0.0312f };

	struct PostProcessFXAA
	{
		bool enable{ true };
		int32_t quality{ 0 };
		int32_t threshold{ 0 };
		int32_t threshold_min{ 0 };
	};

	struct PostProcessDOF
	{
		bool enable{ false };
		float blur_scale{ 1.f };
		float blur_strength{ 1.f };
		int32_t bokeh_samples{ 5 };
		int32_t bokeh_poly{ 8 };
		float focus_point{ 8.24f };
		float near_field{ 0.3f };
		float near_transition{ 0.125f };
		float far_field{ 8.f };
		float far_transition{ 3.f };
	};

	struct PostProcessBloom
	{
		bool enable{ false };
		float threshold{ 1.f };
		float filter_radius{ 0.005f };
		float strength{ 0.3f };
	};

	struct PostProcessCA
	{
		bool enable{ false };
		float distortion{ 2.2f };
		int32_t iterations{ 12 };
	};

	struct PostProcessVignette
	{
		bool enable{ false };
		float inner{ 0.15f };
		float outer{ 1.35f };
		float opacity{ 0.85f };
	};

	struct PostProcessTonemap
	{
		bool enable{ true };
		bool adaptive{ false };
		float gamma{ 2.2f };
		float exposure{ 0.5f };
		float white_point{ 4.f };
		float lum_min{ -5.f };
		float lum_max{ 3.f };
		float tau{ 1.1f };
	};

	struct PostProcessFilmGrain
	{
		bool enable{ false };
		float amount{ 0.01f };
	};

	struct PostProcessMotionBlur
	{
		bool enable{ false };
		float scele{ 1.f };
		int32_t max_samples{ 16 };
		float max_velocity{ 30.f };
		float threshold{ 0.0001f };
	};

	struct PostProcessData
	{
		// fxaa
		PostProcessFXAA fxaa;
		// DOF
		PostProcessDOF dof;
		// Bloom
		PostProcessBloom bloom;
		// Chromatic aberration
		PostProcessCA aberration;
		// Vignette
		PostProcessVignette vignette;
		// Film grain
		PostProcessFilmGrain filmgrain;
		// Tonemapping
		PostProcessTonemap tonemap;
		// Motion blur
		PostProcessMotionBlur motionblur;
	};
}

// TODO: Add other fields
REFL_TYPE(engine::render::PostProcessFXAA, engine::attr::Name("FXAA"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_END

REFL_TYPE(engine::render::PostProcessDOF, engine::attr::Name("Depth Of Field"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_FIELD(blur_scale, engine::attr::Range(0.1f, 5.0f), engine::attr::Name("Bokeh Scale"))
REFL_FIELD(blur_strength, engine::attr::Range(0.1f, 5.0f), engine::attr::Name("Bokeh Strength"))
REFL_FIELD(bokeh_samples, engine::attr::Range(1, 32), engine::attr::Name("Bokeh Sample Count"))
REFL_FIELD(bokeh_poly, engine::attr::Range(3, 16), engine::attr::Name("Bokeh Polygon"))
REFL_FIELD(focus_point, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Focus Point"))
REFL_FIELD(near_field, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Near Field"))
REFL_FIELD(near_transition, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Near Transmission"))
REFL_FIELD(far_field, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Far Field"))
REFL_FIELD(far_transition, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Far Transmission"))
REFL_END

REFL_TYPE(engine::render::PostProcessBloom, engine::attr::Name("Bloom"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_FIELD(threshold, engine::attr::Range(0.1f, 1.0f), engine::attr::Name("Threshold"))
REFL_FIELD(filter_radius, engine::attr::Range(0.01f, 10.0f), engine::attr::Name("Filter Radius"))
REFL_FIELD(strength, engine::attr::Range(0.1f, 2.0f), engine::attr::Name("Strength"))
REFL_END

REFL_TYPE(engine::render::PostProcessCA, engine::attr::Name("Barell Chromatic Aberration"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_FIELD(distortion, engine::attr::Range(0.1f, 5.0f), engine::attr::Name("Distortion Amount"))
REFL_FIELD(iterations, engine::attr::Range(1, 24), engine::attr::Name("Iteration Count"))
REFL_END

REFL_TYPE(engine::render::PostProcessVignette, engine::attr::Name("Vignette"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_FIELD(inner, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Inner"))
REFL_FIELD(outer, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Outer"))
REFL_FIELD(opacity, engine::attr::Range(0.1f, 1.f), engine::attr::Name("Border Opacity"))
REFL_END

REFL_TYPE(engine::render::PostProcessFilmGrain, engine::attr::Name("Film Grain"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_FIELD(amount, engine::attr::Range(0.1f, 1.0f), engine::attr::Name("Amount"))
REFL_END

REFL_TYPE(engine::render::PostProcessTonemap, engine::attr::Name("Tonemapping"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_FIELD(adaptive, engine::attr::Name("Adaptive"))
REFL_FIELD(gamma, engine::attr::Range(0.1f, 5.0f), engine::attr::Name("Gamma"))
REFL_FIELD(exposure, engine::attr::Range(-5.0f, 5.0f), engine::attr::Name("Exposure"))
REFL_FIELD(white_point, engine::attr::Range(0.01f, INFINITY), engine::attr::Name("White Point"))
REFL_FIELD(lum_min, engine::attr::Range(-INFINITY, INFINITY), engine::attr::Name("Luma Min"))
REFL_FIELD(lum_max, engine::attr::Range(-INFINITY, INFINITY), engine::attr::Name("Luma Max"))
REFL_FIELD(tau, engine::attr::Range(0.1f, 10.0f), engine::attr::Name("Adaptation Speed"))
REFL_END

REFL_TYPE(engine::render::PostProcessMotionBlur, engine::attr::Name("Motion Blur"))
REFL_FIELD(enable, engine::attr::Name("Enable"))
REFL_FIELD(scele, engine::attr::Range(0.1f, 5.0f), engine::attr::Name("Blur Scale"))
REFL_FIELD(max_samples, engine::attr::Range(1, 24), engine::attr::Name("Max Samples"))
REFL_FIELD(max_velocity, engine::attr::Range(0.1f, INFINITY), engine::attr::Name("Max Velocity"))
REFL_FIELD(threshold, engine::attr::Range(0.1f, 5.0f), engine::attr::Name("Threshold"))
REFL_END