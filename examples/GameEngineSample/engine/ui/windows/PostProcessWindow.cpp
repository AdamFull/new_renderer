#include "PostProcessWindow.h"

#include "../ImGuiElements.h"

#include "../../rendering/Globals.h"

namespace engine
{
	void PostProcessWindow::Create()
	{

	}

	void PostProcessWindow::__draw(float delta_time)
	{
		using namespace render;

		if (ImGui::CollapsingHeader("FXAA", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			static const char* quality_labels_5[]{ "best", "high", "good", "low", "worse" };
			static const char* quality_labels_3[]{ "best", "good", "low" };

			ImGui::GCheckbox("enable", &postprocess.fxaa.enable);
			ImGui::GCombo("quality", &postprocess.fxaa.quality, quality_labels_5, 5);
			ImGui::GCombo("threshold", &postprocess.fxaa.threshold, quality_labels_5, 5);
			ImGui::GCombo("threshold min", &postprocess.fxaa.threshold_min, quality_labels_3, 3);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Depth of field", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::GCheckbox("enable", &postprocess.dof.enable);
			ImGui::GDragFloat<1>("blur scale", &postprocess.dof.blur_scale, 0.1f, 0.1f, 5.f);
			ImGui::GDragFloat<1>("blur strength", &postprocess.dof.blur_strength, 0.1f, 0.1f, 5.f);
			ImGui::Separator();
			ImGui::GDragInt<1>("sample count", &postprocess.dof.bokeh_samples, 1.f, 1, 32);
			ImGui::GDragInt<1>("polygon count", &postprocess.dof.bokeh_poly, 1.f, 3, 16);
			ImGui::GDragFloat<1>("focus point", &postprocess.dof.focus_point, 0.1f, 0.1f, INFINITY);
			ImGui::GDragFloat<1>("near field", &postprocess.dof.near_field, 0.1f, 0.1f, postprocess.dof.far_field);
			ImGui::GDragFloat<1>("near transition", &postprocess.dof.near_transition, 0.1f, 0.1f, INFINITY);
			ImGui::GDragFloat<1>("far field", &postprocess.dof.far_field, 0.1f, postprocess.dof.near_field, INFINITY);
			ImGui::GDragFloat<1>("far transition", &postprocess.dof.far_transition, 0.1f, 0.1f, INFINITY);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::GCheckbox("enable", &postprocess.bloom.enable);
			ImGui::GDragFloat<1>("threshold", &postprocess.bloom.threshold, 0.1f, 0.1f, 2.f);
			ImGui::GDragFloat<1>("filter radius", &postprocess.bloom.filter_radius, 0.0001f, 0.0001f, 10.f);
			ImGui::GDragFloat<1>("strength", &postprocess.bloom.strength, 0.1f, 0.1f, 2.f);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Chromatic aberration", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::GCheckbox("enable", &postprocess.aberration.enable);
			ImGui::GDragFloat<1>("distortion", &postprocess.aberration.distortion, 0.1f, 0.1f, 5.f);
			ImGui::GDragInt<1>("iterations", &postprocess.aberration.iterations, 1.f, 1, 24);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Vignette", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::GCheckbox("enable", &postprocess.vignette.enable);
			ImGui::GDragFloat<1>("inner", &postprocess.vignette.inner, 0.1f, 0.1f, postprocess.vignette.outer);
			ImGui::GDragFloat<1>("outer", &postprocess.vignette.outer, 0.1f, postprocess.vignette.inner, INFINITY);
			ImGui::GDragFloat<1>("opacity", &postprocess.vignette.outer, 0.01f, 0.01f, 1.f);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Film grain", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::GCheckbox("enable", &postprocess.filmgrain.enable);
			ImGui::GDragFloat<1>("amount", &postprocess.filmgrain.amount, 0.1f, 0.1f, 1.f);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Tonemapping", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::GCheckbox("enable", &postprocess.tonemap.enable);
			ImGui::GCheckbox("adaptive", &postprocess.tonemap.adaptive);
			ImGui::GDragFloat<1>("gamma", &postprocess.tonemap.gamma, 0.1f, 0.1f, 5.f);
			ImGui::GDragFloat<1>("exposure", &postprocess.tonemap.exposure, 0.1f, -5.f, 5.f);
			ImGui::GDragFloat<1>("white point", &postprocess.tonemap.whitePoint, 0.1f, 0.1f, INFINITY);
			ImGui::GDragFloat<1>("min luminance", &postprocess.tonemap.lumMin, 0.1f, -INFINITY, postprocess.tonemap.lumMax);
			ImGui::GDragFloat<1>("max luminance", &postprocess.tonemap.lumMax, 0.1f, postprocess.tonemap.lumMin, INFINITY);
			ImGui::GDragFloat<1>("adaptation speed", &postprocess.tonemap.tau, 0.1f, 0.1f, 5.f);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Motion blur", ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			ImGui::GCheckbox("enable", &postprocess.motionblur.enable);
			ImGui::GDragFloat<1>("scale", &postprocess.motionblur.scele, 0.1f, 0.1f, 5.f);
			ImGui::GDragInt<1>("iterations", &postprocess.motionblur.max_samples, 1.f, 1, 24);
			ImGui::GDragFloat<1>("max velocity", &postprocess.motionblur.max_velocity, 0.1f, 0.1f, INFINITY);
			ImGui::GDragFloat<1>("threshold", &postprocess.motionblur.threshold, 0.0001f, 0.0001f, 5.f);
			ImGui::Separator();
		}
	}
}