#pragma once

#include "VariableEdit.h"

namespace engine
{
	class CUICheckBox : public IUIVariableEdit<bool>
	{
	public:
		CUICheckBox(const char* label, bool& value)
		{
			m_label = label;
			m_value = &value;
		}

		void OnGui(f32 dt) override
		{
			if (m_onEditing && ImGui::Checkbox(m_label.c_str(), m_value))
				m_onEditing(*m_value);

			UpdateStates();
		}
	};
}