#include "Window.h"

#include <imgui.h>

using namespace engine;

CUIWindow::CUIWindow(const char* label, bool enabled)
{
	m_label = label;
	m_bIsOpen = enabled;
}

void CUIWindow::OnInit()
{

}

void CUIWindow::PushElement(IUIElement* element)
{
	m_vElements.emplace_back(element);
}

void CUIWindow::OnGui(f32 dt)
{
	if (ImGui::Begin(m_label.c_str(), &m_bIsOpen))
	{
		for (auto& element : m_vElements)
			element->OnGui(dt);

		ImGui::End();
	}
}