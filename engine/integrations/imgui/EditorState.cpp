#include "EditorState.h"

#include <entt/entt.hpp>

using namespace engine;

void CEditorState::SetSelection(entt::entity entity)
{
	m_vSelected.clear();
	m_vSelected.emplace_back(entity);
}

void CEditorState::AddSelection(entt::entity entity)
{
	m_vSelected.emplace_back(entity);
}

void CEditorState::DeselectAll()
{
	m_vSelected.clear();
}

void CEditorState::DeselectOne(entt::entity entity)
{
	auto found = std::remove(m_vSelected.begin(), m_vSelected.end(), entity);
	m_vSelected.erase(found);
}

bool CEditorState::IsSelected(entt::entity entity)
{
	auto found = std::find(m_vSelected.begin(), m_vSelected.end(), entity);
	return found != m_vSelected.end();
}

entt::entity CEditorState::GetSelection()
{
	if(!m_vSelected.empty())
		return m_vSelected.back();

	return entt::null;
}

const utl::vector<entt::entity>& CEditorState::GetAllSelections()
{
	return m_vSelected;
}