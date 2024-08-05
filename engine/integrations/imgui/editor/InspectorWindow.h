#pragma once

#include "EditorWindow.h"
#include "InspectorDrawer.h"
#include <engine/world/ecs/components/components.h>

namespace engine
{
	class CInspectorWindow : public IEditorWindow
	{
	public:
		CInspectorWindow(const std::string& sname) { m_name = sname; }
		virtual ~CInspectorWindow() override = default;

		void Create() override;
	private:
		CInspectorDrawer m_drawer{};
		void __draw(float fDt) override;
	};
}