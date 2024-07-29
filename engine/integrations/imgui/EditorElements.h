#pragma once

#include "EditorTraits.h"
#include "ImGuiElements.h"

namespace engine
{
	enum class EComponentManipulationResult
	{
		eNone,
		eEdited,
		eDelete
	};

	template<typename _Ty, typename _OnEdit>
	EComponentManipulationResult DefaultComponentEdit(const std::string& name, _OnEdit&& edit)
	{
		EComponentManipulationResult result{ EComponentManipulationResult::eNone };

		ImGui::PushID(name.c_str());

		auto name_with_icon = std::format("{} {}", IconByComponentType_v<_Ty>, name);
		if (ImGui::CollapsingHeader(name_with_icon.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			edit();
			result = EComponentManipulationResult::eEdited;
		}

		ImGui::PopID();
		ImGui::Separator();

		return result;
	}

	template<typename _Ty, typename _OnEdit>
	EComponentManipulationResult ComponentEdit(const std::string& name, _OnEdit&& edit)
	{
		EComponentManipulationResult result{ EComponentManipulationResult::eNone };

		ImGui::PushID(name.c_str());
		auto pre_header = ImGui::GetCursorPos();
		ImVec2 arter_content;

		auto name_with_icon = std::format("{} {}", IconByComponentType_v<_Ty>, name);
		if (ImGui::CollapsingHeader(name_with_icon.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
		{
			edit();
			arter_content = ImGui::GetCursorPos();

			result = EComponentManipulationResult::eEdited;
		}

		auto padding = ImGui::GetStyle().WindowPadding;
		pre_header.x = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(ICON_MDI_DELETE).x + padding.x / 2.f;
		ImGui::SetCursorPos(pre_header);
		if (ImGui::Button(ICON_MDI_DELETE))
			result = EComponentManipulationResult::eDelete;

		ImGui::SetCursorPos(arter_content);

		ImGui::PopID();
		ImGui::Separator();

		return result;
	}
}