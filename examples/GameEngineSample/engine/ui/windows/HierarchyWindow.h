#pragma once

#include "EditorWindow.h"

#include <entt/entt.hpp>

namespace engine
{
	class HierarchyWindow : public IEditorWindow
	{
	public:
		HierarchyWindow(std::string_view sname) { m_name = sname; }
		virtual ~HierarchyWindow() override = default;

		void Create() override {}
	private:
		void __draw(float fDt) override;
		void BuildHierarchy(const entt::entity& entity);

		entt::entity m_currentSelection{ entt::null };
		entt::entity m_copySelection{ entt::null };
	};
}