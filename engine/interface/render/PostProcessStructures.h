#pragma once

namespace engine
{
	namespace render
	{
		struct FPostProcessDOF
		{
			bool enable{ true };
			f32 blur_scale{ 1.f };
			f32 blur_strength{ 1.f };
			i32 bokeh_samples{ 5 };
			f32 bokeh_poly{ 8.f };
			f32 focus_point{ 8.24f };
			f32 near_field{ 0.3f };
			f32 near_transition{ 0.125f };
			f32 far_field{ 8.f };
			f32 far_transition{ 3.f };
		};

		struct FPostProcessBloom
		{
			bool enable{ true };
			f32 threshold{ 1.f };
			f32 filter_radius{ 0.005f };
			f32 strength{ 0.3f };
		};

		struct FPostProcessCA
		{
			bool enable{ false };
			f32 distortion{ 2.2f };
			i32 iterations{ 12 };
		};

		struct FPostProcessVignette
		{
			bool enable{ true };
			f32 inner{ 0.15f };
			f32 outer{ 1.35f };
			f32 opacity{ 0.85f };
		};

		struct FPostProcessTonemap
		{
			bool enable{ true };
			bool adaptive{ true };
			f32 gamma{ 2.2f };
			f32 exposure{ 4.5f };
			f32 whitePoint{ 4.f };
			f32 lumMin{ -5.f };
			f32 lumMax{ 3.f };
			f32 tau{ 1.1f };
		};

		struct FPostProcessFilmGrain
		{
			bool enable{ false };
			f32 amount{ 0.01f };
		};

		struct FPostProcessMotionBlur
		{
			bool enable{ true };
			f32 scele{ 1.f };
			i32 max_samples{ 16 };
			f32 max_velocity{ 30.f };
			f32 threshold{ 0.0001f };
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
			// Motion blur
			FPostProcessMotionBlur motionblur;
		};
	}
}