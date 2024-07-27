#include "EditorWindow.h"

#include <imgui.h>

using namespace engine;

void IEditorWindow::Draw(float fDt)
{
	if (m_bIsOpen)
	{
		if (ImGui::Begin(m_name.c_str(), &m_bIsOpen))
		{
			__draw(fDt);
			ImGui::End();
		}
	}
}