#include "VerticalLayout.h"

//#include <imgui_stacklayout.h>

using namespace engine;

void CUIVerticalLayout::OnInit()
{

}

void CUIVerticalLayout::PushElement(IUIElement* element)
{
	m_vElements.emplace_back(element);
}

void CUIVerticalLayout::OnGui(f32 dt)
{
	//ImGui::BeginVertical(this, m_size, m_align);

	for (auto& element : m_vElements)
		element->OnGui(dt);

	//ImGui::EndVertical();
}