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
		InspectorDrawer::draw_big(postprocess.fxaa, ImGuiTreeNodeFlags_AllowItemOverlap);
		InspectorDrawer::draw_big(postprocess.dof, ImGuiTreeNodeFlags_AllowItemOverlap);
		InspectorDrawer::draw_big(postprocess.bloom, ImGuiTreeNodeFlags_AllowItemOverlap);
		InspectorDrawer::draw_big(postprocess.aberration, ImGuiTreeNodeFlags_AllowItemOverlap);
		InspectorDrawer::draw_big(postprocess.vignette, ImGuiTreeNodeFlags_AllowItemOverlap);
		InspectorDrawer::draw_big(postprocess.filmgrain, ImGuiTreeNodeFlags_AllowItemOverlap);
		InspectorDrawer::draw_big(postprocess.tonemap, ImGuiTreeNodeFlags_AllowItemOverlap);
		InspectorDrawer::draw_big(postprocess.motionblur, ImGuiTreeNodeFlags_AllowItemOverlap);

		const char* culling_labels[4] = { "Freeze culling", "Enable frustum culling", "Enable occlusion culling", "Enable lod selection" };

		uint32_t culling_flags_int = static_cast<uint32_t>(culling_flags);
		if (ImGui::GFlagEdit("Culling flags", &culling_flags_int, culling_labels, 4)) {
			is_culling_freezed = false;
			culling_flags = culling_flags_int;
		}
	}
}