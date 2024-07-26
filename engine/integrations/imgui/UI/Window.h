#pragma once

#include <engine/interface/EngineGui.h>

namespace engine
{
	class CUIWindow final : public IUIElement
	{
	public:
		CUIWindow(const char* label, bool enabled = true);

		void OnInit() override;
		void PushElement(IUIElement* element) override;
		void OnGui(f32 dt) override;
	private:
		utl::vector<IUIElement*> m_vElements{};
		bool m_bIsOpen{ true };
	};
}