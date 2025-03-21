#pragma once

#include <edge/util/utl_enum.hpp>
#include "Attributes.h"
#include "ImGuiElements.h"

namespace engine
{
	template<typename T>
	struct ElementCount { static constexpr std::size_t value = 1; };

	template<>
	struct ElementCount<glm::vec2> { static constexpr std::size_t value = 2; };
	template<>
	struct ElementCount<glm::vec3> { static constexpr std::size_t value = 3; };
	template<>
	struct ElementCount<glm::vec4> { static constexpr std::size_t value = 4; };
	template<>
	struct ElementCount<glm::quat> { static constexpr std::size_t value = 4; };

	class InspectorDrawer
	{
	public:
		template<typename _Member, typename _Ty, glm::qualifier _Q, size_t _Count>
		bool PropertyEdit(_Member member, const char* field_name, glm::vec<_Count, _Ty, _Q>& val)
		{
			attr::Range<_Ty> range;
			if constexpr (has_attribute<attr::Range>(member))
				range = get_attribute<attr::Range>(member);

			auto* ptr = reinterpret_cast<_Ty*>(&val);

			if constexpr (has_attribute<attr::SliderField>(member))
				return ImGui::Slider<_Ty, _Count>(field_name, ptr, range.min, range.max);
			else if constexpr (has_attribute<attr::InputField>(member))
				return ImGui::Input<_Ty, _Count>(field_name, ptr, range.min, range.max);
			else if constexpr(has_attribute<attr::ColorField>(member))
				return ImGui::GColorEdit3(field_name, ptr);

			return ImGui::Drag<_Ty, _Count>(field_name, ptr, range.min, range.max, 0.01f);
		}

		template<typename _Member, typename _Ty, glm::qualifier _Q>
		bool PropertyEdit(_Member member, const char* field_name, glm::qua<_Ty, _Q>& val)
		{
			auto angles = glm::degrees(glm::eulerAngles(val));
			if (PropertyEdit(member, field_name, angles))
			{
				val = glm::qua<_Ty, _Q>(glm::radians(angles));
				return true;
			}
			return false;
		}

		template<typename _Member, typename _Ty>
		bool PropertyEdit(_Member member, const char* field_name, _Ty& val) 
			requires std::is_fundamental_v<_Ty> && std::is_arithmetic_v<_Ty> && !std::is_same_v<_Ty, bool>
		{
			attr::Range<_Ty> range;
			if constexpr (has_attribute<attr::Range>(member))
				range = get_attribute<attr::Range>(member);

			if constexpr (has_attribute<attr::SliderField>(member))
				return ImGui::Slider<_Ty, 1>(field_name, &val, range.min, range.max);
			else if constexpr (has_attribute<attr::InputField>(member))
				return ImGui::Input<_Ty, 1>(field_name, &val, range.min, range.max);

			return ImGui::Drag<_Ty, 1>(field_name, &val, range.min, range.max, 0.01f);
		}

		template<typename _Ty>
		bool PropertyEdit(const char* field_name, _Ty& val)
			requires std::is_same_v<_Ty, bool>
		{
			return ImGui::GCheckbox(field_name, &val);
		}

		template<typename _Element>
		bool PropertyEdit(int idx, _Element& element)
		{
			auto label = std::format("Element {}", idx);
			if constexpr (std::is_arithmetic_v<_Element>)
				return ImGui::Input<_Element, 1>(label.c_str(), &element, 1, 1, ImGui::FormatByType_v<_Element>);
			else
			{
				if (ImGui::TreeNode(label.c_str()))
				{
					return Draw(element);
					ImGui::TreePop();
				}
			}
		}

		template<typename _Ty>
		bool PropertyEdit(const char* field_name, std::vector<_Ty>& container)
		{
			ImGui::PushID(field_name);

			if (ImGui::TreeNodeEx(field_name, ImGuiTreeNodeFlags_DefaultOpen))
			{
				int size = static_cast<int>(container.size());
				if (ImGui::GDragInt<1>("Size", &size, 1, 1))
				{
					if (size < 0) size = 0;

					if (size != static_cast<int>(container.size()))
						container.resize(size);
				}

				for (int i = 0; i < size; ++i)
					PropertyEdit(i, container[i]);

				if (ImGui::Button("Add"))
					container.emplace_back(_Ty{});

				ImGui::TreePop();
			}
			ImGui::PopID();

			return false;
		}



		template<typename _Enum>
		bool PropertyEdit(const char* field_name, _Enum& val) requires std::is_enum_v<_Enum>
		{
			if constexpr (edge::has_enum_metadata<_Enum>)
			{
				std::string_view selected;
				auto it = std::find_if(edge::enum_metadata<_Enum>.begin(), edge::enum_metadata<_Enum>.end(), [val](auto& pair) { return pair.first == val; });
				if (it != edge::enum_metadata<_Enum>.end())
					selected = it->second;

				bool changed = false;
				if (ImGui::GBeginTable(field_name))
				{
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::BeginCombo(field_name, selected.data()))
					{
						for (auto& [entry, entryString] : edge::enum_metadata<_Enum>)
						{
							bool isSelected = val == entry;
							if (ImGui::Selectable(entryString, isSelected))
							{
								val = entry;
								changed = true;
							}

							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::GEndTable();
				}

				return changed;
			}
			else
				return ImGui::InputInt<1>(field_name, reinterpret_cast<std::underlying_type_t<_Enum>*>(&val));

			return false;
		}

		template<typename _Member, typename _Ty>
		bool PropertyEdit(_Member member, const char* field_name, _Ty& obj) 
			requires conc::Reflectable<_Ty>
		{
			return Draw(obj);
		}

		template<typename _Member, typename _Ty>
		bool PropertyEdit(_Member member, const char* field_name, _Ty& obj)
		{
			return PropertyEdit(field_name, obj);
		}

		template<typename _Ty>
		bool Draw(_Ty& obj)
			requires conc::Reflectable<_Ty>
		{
			bool changed{ false };
			// Find all displayable members
			auto members = filter(refl::reflect<_Ty>().members, [](auto member) { return !has_attribute<attr::HiddenField>(member); });

			auto fields = filter(members, [](auto member) { return is_field(member); });
			for_each(fields, [&](auto member)
				{
					changed |= PropertyEdit(member, member.name.c_str(), member(obj));
				});

			return changed;
		}
	};
}