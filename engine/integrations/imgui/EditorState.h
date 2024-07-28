#pragma once

#include <design_pattern/pattern.h>
#include <entt/fwd.hpp>

namespace engine
{
	class CEditorState : public utl::pattern::singleton<CEditorState>
	{
	public:
		void SetSelection(entt::entity entity);
		void AddSelection(entt::entity entity);

		void DeselectAll();
		void DeselectOne(entt::entity entity);

		bool IsSelected(entt::entity entity);

		entt::entity GetSelection();
		const utl::vector<entt::entity>& GetAllSelections();
	private:
		utl::vector<entt::entity> m_vSelected;
	};
}