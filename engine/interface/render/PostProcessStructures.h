#pragma once

namespace engine
{
	namespace render
	{
		struct FPostProcessDOF
		{
			bool enable{ false };
			float blur_scale{ 1.f };
			float blur_strength{ 1.f };
			int bokeh_samples{ 5 };
			float bokeh_poly{ 8.f };
			float focus_point{ 8.24f };
			float near_field{ 0.3f };
			float near_transition{ 0.125f };
			float far_field{ 8.f };
			float far_transition{ 3.f };
		};

		struct FPostProcessBloom
		{
			bool enable{ false };
			float threshold{ 1.f };
			float filter_radius{ 0.005f };
			float strength{ 0.04f };
		};

		struct FPostProcessCA
		{
			bool enable{ false };
			float distortion{ 2.2f };
			int iterations{ 12 };
		};

		struct FPostProcessVignette
		{
			bool enable{ false };
			float inner{ 0.15f };
			float outer{ 1.35f };
			float opacity{ 0.85f };
		};

		struct FPostProcessTonemap
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

		struct FPostProcessFilmGrain
		{
			bool enable{ false };
			float amount{ 0.01f };
		};

		struct FPostProcess
		{
			// DOF
			FPostProcessDOF dof;
			// Bloom
			FPostProcessBloom bloom;
			// Chromatic aberration
			FPostProcessCA aberration;
			// Vignette
			FPostProcessVignette vignette;
			// Film grain
			FPostProcessFilmGrain filmgrain;
			// Tonemapping
			FPostProcessTonemap tonemap;
		};
	}
}