#include "EditorState.h"

#include <entt/entt.hpp>

using namespace engine;

void EditorState::SetSelection(entt::entity entity)
{
	m_vSelected.clear();
	m_vSelected.emplace_back(entity);
}

void EditorState::AddSelection(entt::entity entity)
{
	m_vSelected.emplace_back(entity);
}

void EditorState::DeselectAll()
{
	m_vSelected.clear();
}

void EditorState::DeselectOne(entt::entity entity)
{
	auto found = std::remove(m_vSelected.begin(), m_vSelected.end(), entity);
	m_vSelected.erase(found);
}

bool EditorState::IsSelected(entt::entity entity)
{
	auto found = std::find(m_vSelected.begin(), m_vSelected.end(), entity);
	return found != m_vSelected.end();
}

entt::entity EditorState::GetSelection()
{
	if(!m_vSelected.empty())
		return m_vSelected.back();

	return entt::null;
}

const std::vector<entt::entity>& EditorState::GetAllSelections()
{
	return m_vSelected;
}