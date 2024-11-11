#pragma once

#include <standart_library.h>

namespace engine
{
	namespace render
	{
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
			bool adaptive{ true };
			float gamma{ 2.2f };
			float exposure{ 4.5f };
			float whitePoint{ 4.f };
			float lumMin{ -5.f };
			float lumMax{ 3.f };
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
}