#pragma once

#include <engine/integrations/imgui/Attributes.h>
#include <engine/integrations/imgui/ImGuiElements.h>

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

	class CInspectorDrawer
	{
	public:
		template<typename _Ty, size_t _Count>
		bool SliderEdit(const char* field_name, _Ty* obj, const _Ty& min, const _Ty& max)
		{
			return ImGui::Slider<_Ty, _Count>(field_name, obj, attr.min, attr.max);
		}

		template<typename _Ty, size_t _Count>
		bool DragEdit(const char* field_name, _Ty* obj, const _Ty& min, const _Ty& max)
		{
			return ImGui::Drag<_Ty, _Count>(field_name, obj, min, max, 1.f);
		}

		template<typename _Ty, size_t _Count>
		bool InputEdit(const char* field_name, _Ty* obj, const _Ty& min, const _Ty& max)
		{
			return ImGui::Input<_Ty, _Count>(field_name, obj, min, max);
		}

		template<typename _Ty, glm::qualifier _Q, template<typename> class FieldType>
		bool PropertyEdit(const char* field_name, glm::qua<_Ty, _Q>& obj, FieldType<_Ty>& attr)
		{
			glm::vec<3ull, _Ty, _Q> angles = glm::degrees(glm::eulerAngles(obj));
			bool edited{ false };

			auto* ptr = reinterpret_cast<_Ty*>(&angles);
			if constexpr (std::is_same_v<FieldType<_Ty>, attr::SliderField<_Ty>>)
				edited = SliderEdit<_Ty, 3>(field_name, ptr, attr.min, attr.max);
			else if constexpr (std::is_same_v<FieldType<_Ty>, attr::DragField<_Ty>>)
				edited = DragEdit<_Ty, 3>(field_name, ptr, attr.min, attr.max);
			else if constexpr (std::is_same_v<FieldType<_Ty>, attr::InputField<_Ty>>)
				edited = InputEdit<_Ty, 3>(field_name, ptr, attr.min, attr.max);

			if (edited)
			{
				obj = glm::qua<_Ty, _Q>(glm::radians(angles));
				return true;
			}
			return false;
		}

		template<size_t _Count, typename _Ty, glm::qualifier _Q, template<typename> class FieldType>
		bool PropertyEdit(const char* field_name, glm::vec<_Count, _Ty, _Q>& obj, FieldType<_Ty>& attr)
		{
			auto* ptr = reinterpret_cast<_Ty*>(&obj);
			if constexpr (std::is_same_v<FieldType<_Ty>, attr::SliderField<_Ty>>)
				return SliderEdit<_Ty, _Count>(field_name, ptr, attr.min, attr.max);
			else if constexpr (std::is_same_v<FieldType<_Ty>, attr::DragField<_Ty>>)
				return DragEdit<_Ty, _Count>(field_name, ptr, attr.min, attr.max);
			else if constexpr (std::is_same_v<FieldType<_Ty>, attr::InputField<_Ty>>)
				return InputEdit<_Ty, _Count>(field_name, ptr, attr.min, attr.max);
			else
				return false;
		}

		template<typename _Ty, template<typename> class FieldType>
		bool PropertyEdit(const char* field_name, _Ty& obj, FieldType<_Ty>& attr) requires std::is_arithmetic_v<_Ty>
		{
			if constexpr (std::is_same_v<FieldType<_Ty>, attr::SliderField<_Ty>>)
				return SliderEdit<_Ty, 1>(field_name, &obj, attr.min, attr.max);
			else if constexpr (std::is_same_v<FieldType<_Ty>, attr::DragField<_Ty>>)
				return DragEdit<_Ty, 1>(field_name, &obj, attr.min, attr.max);
			else if constexpr (std::is_same_v<FieldType<_Ty>, attr::InputField<_Ty>>)
				return InputEdit<_Ty, 1>(field_name, &obj, attr.min, attr.max);
			else
				return false;
		}

		bool PropertyEdit(const char* field_name, bool& val)
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
			if constexpr (HasEnumEntries<_Enum>)
			{
				const char* selected{ nullptr };
				auto it = std::find_if(EnumEntries<_Enum>.begin(), EnumEntries<_Enum>.end(), [val](auto& pair) { return pair.first == val; });
				if (it != EnumEntries<_Enum>.end())
					selected = it->second;
		
				bool changed = false;
				if (ImGui::BeginCombo(field_name, selected))
				{
					for (auto& [entry, entryString] : EnumEntries<_Enum>)
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
				return changed;
			}
			//else
			//	return ImGui::InputInt<1>(field_name, reinterpret_cast<std::underlying_type_t<_Enum>*>(&val));

			return false;
		}

		template<typename _Member, typename _Ty>
		bool PropertyEdit(_Member member, const char* field_name, _Ty& obj)
		{
			if constexpr (refl::is_reflectable<_Ty>())
				return Draw(obj);
			else if constexpr (has_attribute<attr::SliderField>(member))
			{
				auto attr = get_attribute<attr::SliderField>(member);
				return PropertyEdit(field_name, obj, attr);
			}
			else if constexpr (has_attribute<attr::DragField>(member))
			{
				auto attr = get_attribute<attr::DragField>(member);
				return PropertyEdit(field_name, obj, attr);
			}
			else if constexpr (has_attribute<attr::InputField>(member))
			{
				auto attr = get_attribute<attr::InputField>(member);
				return PropertyEdit(field_name, obj, attr);
			}
			else if constexpr (has_attribute<attr::ColorField>(member))
			{
				return ImGui::GColorEdit3(field_name, reinterpret_cast<f32*>(&obj));
			}
			else
			{
				PropertyEdit(field_name, obj);
			}

			return false;
		}

		template<typename _Ty>
		bool Draw(_Ty& obj)
		{
			if constexpr (refl::is_reflectable<_Ty>())
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

			return false;
			}
	};
}