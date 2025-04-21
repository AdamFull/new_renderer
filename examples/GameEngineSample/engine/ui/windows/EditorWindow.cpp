#include "EditorWindow.h"

#include <edge/util/utl_imgui.hpp>

using namespace engine;

void IEditorWindow::Draw(float fDt)
{
	if (auto window = edge::imui::make_scoped_window(m_name.c_str(), &m_bIsOpen)) {
		__draw(fDt);
	}
}