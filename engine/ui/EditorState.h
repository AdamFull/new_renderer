#pragma once

#include <edge/util/utl_design.h>
#include <entt/fwd.hpp>

namespace engine
{
	class EditorState : public edge::utl::Singleton<EditorState>
	{
	public:
		void SetSelection(entt::entity entity);
		void AddSelection(entt::entity entity);

		void DeselectAll();
		void DeselectOne(entt::entity entity);

		bool IsSelected(entt::entity entity);

		entt::entity GetSelection();
		const std::vector<entt::entity>& GetAllSelections();
	private:
		std::vector<entt::entity> m_vSelected;
	};
}