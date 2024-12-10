#pragma once

#include "EditorWindow.h"
#include "../InspectorDrawer.h"
#include <engine/world/ecs/components/components.h>

namespace engine
{
	class InspectorWindow : public IEditorWindow
	{
	public:
		InspectorWindow(std::string_view sname) { m_name = sname; }
		virtual ~InspectorWindow() override = default;

		void Create() override;
	private:
		InspectorDrawer m_drawer{};
		void __draw(float fDt) override;
	};
}