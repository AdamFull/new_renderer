#pragma once

#include <engine/integrations/imgui/Attributes.h>
#include <engine/integrations/imgui/ImGuiElements.h>

namespace engine
{
	class CInspectorDrawer
	{
	public:
		template<typename _Prop, typename _Ty>
		void PropertyEdit(_Prop& property, const char* name, _Ty& obj)
			requires conc::Reflectable<_Ty>
		{

		}

		template<typename _Ty>
		void Draw(_Ty& obj)
		{
			if constexpr (refl::is_reflectable<_Ty>())
			{
				// Find all displayable members
				auto members = filter(refl::reflect<_Ty>().members, [](auto member) { return !has_attribute<attr::HiddenField>(member); });

				auto fields = filter(members, [](auto member) { return is_field(member); });
				for_each(fields, [&](auto member) { (member, member.name.c_str(), member(obj)); });
			}
		}
	};
}