#pragma once

#include "EditorWindow.h"

#include <entt/entt.hpp>

namespace engine
{
	class CEditorHierarchy : public IEditorWindow
	{
	public:
		CEditorHierarchy(const std::string& sname) { m_name = sname; }
		virtual ~CEditorHierarchy() override = default;

		void Create() override {}
		void __draw(float fDt) override;
	private:
		entt::entity m_currentSelection{ entt::null };
		entt::entity m_copySelection{ entt::null };
		void BuildHierarchy(const entt::entity& entity);
	};
}