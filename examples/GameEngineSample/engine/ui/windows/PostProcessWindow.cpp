#include "PostProcessWindow.h"

#include "../ImGuiElements.h"

#include "../../rendering/Globals.h"
#include "../InspectorDrawer.h"

namespace engine
{
	void PostProcessWindow::Create()
	{

	}

	void PostProcessWindow::__draw(float delta_time)
	{
		using namespace render;
		InspectorDrawer::draw_big(postprocess.fxaa);
		InspectorDrawer::draw_big(postprocess.dof);

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