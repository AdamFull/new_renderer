#include "Button.h"

#include <imgui.h>

using namespace engine;

CUIButton::CUIButton(const char* name, Callback&& onClick)
{
	m_label = name;
	m_onClick = std::move(onClick);
}

void CUIButton::OnInit()
{

}

void CUIButton::OnGui(f32 dt)
{
	if (m_onClick && ImGui::Button(m_label.c_str()))
		m_onClick();
}