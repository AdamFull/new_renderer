#pragma once

#include <reactive/function.h>
#include <engine/interface/EngineGui.h>
#include "Util.h"

#include "VariableEdit.h"

namespace engine
{
	template<typename _Ty>
	class CUIDragScalar : public IUIVariableEdit<_Ty>
	{
	public:
		CUIDragScalar(const char* label, _Ty& value, _Ty valueMin = (_Ty)0, _Ty valueMax = (_Ty)0, const char* format = "") :
			m_format{ format }
		{
			this->m_label = label;
			this->m_value = &value;
			this->m_valueMin = valueMin; 
			this->m_valueMax = valueMax;
		}

		void OnGui(f32 dt) override
		{
			if (this->m_onEditing && ImGui::DragScalar(this->m_label.c_str(), ImGuiDataTypeByType_v<_Ty>, this->m_value, m_changingSpeed, &this->m_valueMin, &this->m_valueMax, m_format.c_str()))
				this->m_onEditing(*this->m_value);

			this->UpdateStates();
		}
	private:
		f32 m_changingSpeed{ 1.f };
		std::string m_format;
	};
}