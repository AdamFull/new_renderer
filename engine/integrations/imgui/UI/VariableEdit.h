#pragma once

#include <engine/interface/EngineGui.h>
#include <imgui_internal.h>

namespace engine
{
	template<typename _Ty>
	class IUIVariableEdit : public IUIElement
	{
	public:
		using Callback = utl::reactive::function<void(const _Ty&)>;

		virtual ~IUIVariableEdit() = default;

		virtual void OnInit() override {}
		virtual void PushElement(IUIElement* element) override {}
		virtual void OnGui(f32 dt) override {}

		virtual void SubscribeOnBeginEdit(Callback&& callback)
		{
			m_onBeginEditing = std::move(callback);
		}

		virtual void SubscribeOnEdit(Callback&& callback)
		{
			m_onEditing = std::move(callback);
		}

		virtual void SubscribeOnEndEdit(Callback&& callback)
		{
			m_onEndEditing = std::move(callback);
		}

	protected:
		virtual void UpdateStates()
		{
			if (m_onBeginEditing && ImGui::IsItemActivated())
				m_onBeginEditing(*m_value);

			if (m_onEndEditing && ImGui::IsItemDeactivatedAfterEdit())
				m_onEndEditing(*m_value);
		}
	protected:
		_Ty* m_value;
		_Ty m_valueMin{ (_Ty)0 };
		_Ty m_valueMax{ (_Ty)0 };

		Callback m_onBeginEditing;
		Callback m_onEditing;
		Callback m_onEndEditing;
	};
}