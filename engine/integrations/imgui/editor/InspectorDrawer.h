#pragma once

#include <engine/integrations/imgui/Attributes.h>
#include <engine/integrations/imgui/ImGuiElements.h>

namespace engine
{
	class CInspectorDrawer
	{
	public:
		template<typename _Ty>
		void Draw(_Ty& obj)
		{
			if constexpr (refl::is_reflectable<_Ty>())
			{
				// Find all displayable members
				auto members = filter(refl::reflect<_Ty>().members, [](auto member) { return !has_attribute<attr::HiddenField>(member); });
			}
		}
	};
}