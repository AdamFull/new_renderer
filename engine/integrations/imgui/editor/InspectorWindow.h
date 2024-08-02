#pragma once

#include "EditorWindow.h"
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
		void __draw(float fDt) override;
	};
}