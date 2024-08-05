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


		template<size_t _Count, typename _Ty, glm::qualifier _Q>
		bool PropertyEdit(const char* field_name, glm::vec<_Count, _Ty, _Q>& obj, attr::SliderField<_Ty>& attr)
		{
			return SliderEdit<_Ty, _Count>(field_name, reinterpret_cast<_Ty*>(&obj), attr.min, attr.max);
		}

		template<size_t _Count, typename _Ty, glm::qualifier _Q>
		bool PropertyEdit(const char* field_name, glm::vec<_Count, _Ty, _Q>& obj, attr::DragField<_Ty>& attr)
		{
			return DragEdit<_Ty, _Count>(field_name, reinterpret_cast<_Ty*>(&obj), attr.min, attr.max);
		}

		template<size_t _Count, typename _Ty, glm::qualifier _Q>
		bool PropertyEdit(const char* field_name, glm::vec<_Count, _Ty, _Q>& obj, attr::InputField<_Ty>& attr)
		{
			return InputEdit<_Ty, _Count>(field_name, reinterpret_cast<_Ty*>(&obj), attr.min, attr.max);
		}


		template<typename _Ty, glm::qualifier _Q>
		bool PropertyEdit(const char* field_name, glm::qua<_Ty, _Q>& obj, attr::SliderField<_Ty>& attr)
		{
			glm::vec<3ull, _Ty, _Q> angles = glm::degrees(glm::eulerAngles(obj));
			if (SliderEdit<_Ty, 3>(field_name, reinterpret_cast<_Ty*>(&angles), attr.min, attr.max))
			{
				obj = glm::qua<_Ty, _Q>(glm::radians(angles));
				return true;
			}
			return false;
		}

		template<typename _Ty, glm::qualifier _Q>
		bool PropertyEdit(const char* field_name, glm::qua<_Ty, _Q>& obj, attr::DragField<_Ty>& attr)
		{
			glm::vec<3ull, _Ty, _Q> angles = glm::degrees(glm::eulerAngles(obj));
			if (DragEdit<_Ty, 3>(field_name, reinterpret_cast<_Ty*>(&angles), attr.min, attr.max))
			{
				obj = glm::qua<_Ty, _Q>(glm::radians(angles));
				return true;
			}
			return false;
		}

		template<typename _Ty, glm::qualifier _Q>
		bool PropertyEdit(const char* field_name, glm::qua<_Ty, _Q>& obj, attr::InputField<_Ty>& attr)
		{
			glm::vec<3ull, _Ty, _Q> angles = glm::degrees(glm::eulerAngles(obj));
			if (InputEdit<_Ty, 3>(field_name, reinterpret_cast<_Ty*>(&angles), attr.min, attr.max))
			{
				obj = glm::qua<_Ty, _Q>(glm::radians(angles));
				return true;
			}
			return false;
		}


		template<typename _Ty>
		bool PropertyEdit(const char* field_name, _Ty& obj, attr::SliderField<_Ty>& attr)
		{
			return SliderEdit<_Ty, 1>(field_name, reinterpret_cast<_Ty*>(&obj), attr.min, attr.max);
		}

		template<typename _Ty>
		bool PropertyEdit(const char* field_name, _Ty& obj, attr::DragField<_Ty>& attr)
		{
			return DragEdit<_Ty, 1>(field_name, reinterpret_cast<_Ty*>(&obj), attr.min, attr.max);
		}

		template<typename _Ty>
		bool PropertyEdit(const char* field_name, _Ty& obj, attr::InputField<_Ty>& attr)
		{
			return InputEdit<_Ty, 1>(field_name, reinterpret_cast<_Ty*>(&obj), attr.min, attr.max);
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

		template<typename _Member, typename _Ty>
		bool PropertyEdit(_Member member, const char* field_name, _Ty& obj)
		{
			if constexpr (refl::is_reflectable<_Ty>())
				return Draw(obj);
			if constexpr (has_attribute<attr::SliderField>(member))
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
			else if constexpr (has_attribute<attr::ToggleField>(member))
			{
				return ImGui::GCheckbox(field_name, &obj);
			}
			else if constexpr (traits::is_vector<_Ty>::value)
			{
				return PropertyEdit(field_name, obj);
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