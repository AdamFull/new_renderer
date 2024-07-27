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
		entt::entity selected_entity{ entt::null };
		entt::entity copy_entity{ entt::null };
		void BuildHierarchy(const entt::entity& entity);
	};
}