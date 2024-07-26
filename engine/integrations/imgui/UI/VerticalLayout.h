#pragma once

#include <engine/interface/EngineGui.h>

namespace engine
{
	class CUIVerticalLayout : public IUIElement
	{
	public:
		void OnInit() override;
		void PushElement(IUIElement* element) override;
		void OnGui(f32 dt) override;
	private:
		utl::vector<IUIElement*> m_vElements{};
		//ImVec2 m_size{ 0.f, 0.f };
		f32 m_align{ -1.f };
	};
}