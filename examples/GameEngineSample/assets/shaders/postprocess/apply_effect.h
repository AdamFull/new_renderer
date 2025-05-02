#pragma once

namespace apply_effect {
	struct PushConstantFXAA {
		int enabled;
		float quality;
		float threshold;
		float threshold_min;
	};

	struct PushConstantMotionBlur {
		int enabled;
		float scale;
		float threshold;
		int max_samples;
	};

	struct PushConstantBloom {
		int enabled;
		float strength;
	};

	struct PushConstantDoF {
		int enabled;
		float near_field;
		float near_transition;
		float far_field;
		float far_transition;
		float focus_point;
	};

	struct PushConstantChromaticAberration {
		int enabled;
		int iterations;
		float distortion;
	};

	struct PushConstantTonemap {
		int enabled;
		int adaptive;
		float white_point;
		float exposure;
		float gamma;
	};

	struct PushConstantGaussianBlur {
		int enabled;
		float scale;
		float strength;
	};

	struct PushConstantFilmGrain {
		int enabled;
		float amount;
	};

	struct PushConstantVignette {
		int enabled;
		float opacity;
		float inner_radius;
		float outer_radius;
	};
}