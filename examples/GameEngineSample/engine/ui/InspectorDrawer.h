#pragma once

#include <edge/util/utl_enum.hpp>
#include "Attributes.h"
#include "ImGuiElements.h"

namespace engine {
	template<typename T> struct is_std_vector : std::false_type {};
	template<typename... Ts> struct is_std_vector<std::vector<Ts...>> : std::true_type {};

	template<typename T> struct is_std_map : std::false_type {};
	template<typename... Ts> struct is_std_map<std::map<Ts...>> : std::true_type {};
	template<typename... Ts> struct is_std_map<std::unordered_map<Ts...>> : std::true_type {};

	template<typename T> struct is_std_container : std::false_type {};
	template<typename... Ts> struct is_std_container<std::vector<Ts...>> : std::true_type {};
	template<typename... Ts> struct is_std_container<std::map<Ts...>> : std::true_type {};
	template<typename... Ts> struct is_std_container<std::unordered_map<Ts...>> : std::true_type {};

	template<typename _Ty> struct is_glm_vec : std::false_type {};
	template<glm::length_t N, typename _Ty, glm::qualifier Q>
	struct is_glm_vec<glm::vec<N, _Ty, Q>> : std::true_type {};

	template<typename _Ty> struct is_glm_quat : std::false_type {};
	template<typename _Ty, glm::qualifier _Q> 
	struct is_glm_quat<glm::qua<_Ty, _Q>> : std::true_type {};

	class InspectorDrawer {
	public:
		// TODO: check ability to use requires with has_attribute
		template<glm::length_t _N, typename _Ty, glm::qualifier _Q, typename _Member>
		bool edit_glm_vec(std::string_view label, glm::vec<_N, _Ty, _Q>& value, _Member member) {
			// Try to get min max values
			attr::Range<_Ty> range;
			if constexpr (has_attribute<attr::Range>(member)) {
				range = get_attribute<attr::Range>(member);
			}
			
			auto* ptr = reinterpret_cast<_Ty*>(&value);

			if constexpr (has_attribute<attr::SliderField>(member)) {
				return ImGui::Slider<_Ty, _N>(label.data(), ptr, range.min, range.max);
			}
			else if constexpr (has_attribute<attr::InputField>(member)) {
				return ImGui::Input<_Ty, _N>(label.data(), ptr, range.min, range.max);
			}
			// TODO: Check that it's vec3 or vec4
			else if constexpr (has_attribute<attr::ColorField>(member)) {
				return ImGui::GColorEdit3(label.data(), ptr);
			}
			
			return ImGui::Drag<_Ty, _N>(label.data(), ptr, range.min, range.max, 0.01f);
		}

		template<typename _Ty, glm::qualifier _Q, typename _Member>
		bool edit_glm_quat(std::string_view label, glm::qua<_Ty, _Q>& value, _Member member) {
			auto angles = glm::degrees(glm::eulerAngles(value));
			if (edit_glm_vec(label, angles, member)) {
				value = glm::qua<_Ty, _Q>(glm::radians(angles));
				return true;
			}
			return false;
		}

		template<typename _Ty, typename _Member>
		bool edit_std_vector(std::string_view label, std::vector<_Ty>& vector, _Member member) {
			bool edited{ false };
			if (ImGui::TreeNodeEx(label.data(), ImGuiTreeNodeFlags_DefaultOpen)) {
				// Add resize field
				int vector_size = static_cast<int>(vector.size());
				if (ImGui::GDragInt<1>("Size", &vector_size, 1, 1)) {
					if (vector_size < 0) {
						vector_size = 0;
					}
				
					if (vector_size != static_cast<int>(vector.size())) {
						vector.resize(vector_size);
					}
				}

				for (int32_t i = 0; i < vector.size(); ++i) {
					ImGui::PushID(i);
					edited |= edit_field(std::format("Element {}", i), vector[i], member);
					ImGui::PopID();
				}

				if (ImGui::Button("Add")) {
					vector.emplace_back();
				}
				ImGui::SameLine();
				if (ImGui::Button("Remove") && !vector.empty()) {
					vector.pop_back();
				}

				ImGui::TreePop();
			}

			return edited;
		}

		// For other cases
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member) {
			return false;
		}

		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member) 
		requires is_std_vector<_Ty>::value {
			return edit_std_vector(label, value, member);
		}

		// For scalar numeric types
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member)
		requires std::is_arithmetic_v<_Ty> && (!std::is_same_v<_Ty, bool>) {
			attr::Range<_Ty> range;
			if constexpr (has_attribute<attr::Range>(member)) {
				range = get_attribute<attr::Range>(member);
			}

			if constexpr (has_attribute<attr::SliderField>(member)) {
				return ImGui::Slider<_Ty, 1>(label.data(), &value, range.min, range.max);
			}
			else if constexpr (has_attribute<attr::InputField>(member)) {
				return ImGui::Input<_Ty, 1>(label.data(), &value, range.min, range.max);
			}
			
			return ImGui::Drag<_Ty, 1>(label.data(), &value, range.min, range.max, 0.01f);
		}

		// For glm vectors support
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member) 
		requires is_glm_vec<_Ty>::value {
			return edit_glm_vec(label, value, member);
		}

		// For glm quaternion
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member)
			requires is_glm_quat<_Ty>::value {
			return edit_glm_quat(label, value, member);
		}

		// For boolean
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member)
			requires std::is_same_v<_Ty, bool> {
			return ImGui::GCheckbox(label.data(), &value);
		}

		// For enums with metadata
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member) 
			requires std::is_enum_v<_Ty> && edge::has_enum_metadata<_Ty> {
			std::string_view selected;
			auto it = std::find_if(edge::enum_metadata<_Ty>.begin(), edge::enum_metadata<_Ty>.end(), [&value](auto& pair) { return pair.first == value; });
			if (it != edge::enum_metadata<_Ty>.end())
				selected = it->second;
			
			bool changed = false;
			if (ImGui::GBeginTable(label.data())) {
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (ImGui::BeginCombo(label.data(), selected.data())) {
					for (auto& [entry, entryString] : edge::enum_metadata<_Ty>) {
						bool isSelected = value == entry;
						if (ImGui::Selectable(entryString, isSelected)) {
							value = entry;
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

		// For enums without metadata
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member)
			requires std::is_enum_v<_Ty> && (!edge::has_enum_metadata<_Ty>) {
			int i = static_cast<int>(value);
			bool r = ImGui::InputInt(label.data(), &i);
			value = static_cast<std::underlying_type_t<_Ty>>(i);
			return r;
		}

		// For reflectible (complicated) types
		template<typename _Ty, typename _Member>
		bool edit_field(std::string_view label, _Ty& value, _Member member) 
			requires conc::Reflectable<_Ty> {
			return draw_small(value);
		}

		template<typename _Comp, typename _Member>
		bool edit_member(_Comp& component, _Member member) {
			std::string_view label{ member.name.c_str() };
			if constexpr (has_attribute<attr::Name>(member)) {
				constexpr auto field_name_attr = get_attribute<attr::Name>(member);
				label = field_name_attr.name;
			}

			ImGui::PushID(&member);

			auto& value = member(component);
			bool edited = edit_field(label, value, member);

			ImGui::PopID();

			return edited;
		}

		template<typename _Ty>
		bool draw_big(_Ty& type) 
			requires conc::Reflectable<_Ty> {
			constexpr auto type_desc = refl::reflect<_Ty>();

			std::string_view type_name{ type_desc.name.c_str() };
			if constexpr (has_attribute<attr::Name>(type_desc)) {
				constexpr auto name_attr = get_attribute<attr::Name>(type_desc);
				type_name = name_attr.name;
			}

			// Find all displayable members
			auto members = filter(type_desc.members, [](auto member) {
				return !has_attribute<attr::HiddenField>(member);
				});

			bool changed{ false };
			if (ImGui::CollapsingHeader(type_name.data(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap)) {
				auto fields = filter(members, [](auto member) { return is_field(member); });
				for_each(fields, [&](auto member) {
						changed |= edit_member(type, member);
					});
				ImGui::Separator();
			}

			return changed;
		}

		template<typename _Ty>
		bool draw_small(_Ty& type)
			requires conc::Reflectable<_Ty> {
			constexpr auto type_desc = refl::reflect<_Ty>();

			std::string_view type_name{ type_desc.name.c_str() };
			if constexpr (has_attribute<attr::Name>(type_desc)) {
				constexpr auto name_attr = get_attribute<attr::Name>(type_desc);
				type_name = name_attr.name;
			}

			// Find all displayable members
			auto members = filter(type_desc.members, [](auto member) {
				return !has_attribute<attr::HiddenField>(member);
				});

			bool changed{ false };
			if (ImGui::TreeNode(type_name.data())) {
				auto fields = filter(members, [](auto member) { return is_field(member); });
				for_each(fields, [&](auto member) {
					changed |= edit_member(type, member);
					});
				ImGui::Separator();
				ImGui::TreePop();
			}

			return changed;
		}
	};
}