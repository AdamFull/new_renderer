#pragma once

#include <engine/interface/EngineGui.h>
#include <imgui.h>

namespace engine
{
	class CUIButton : public IUIElement
	{
	public:
		using Callback = utl::reactive::function<void()>;

		CUIButton(const char* name, Callback&& onClick);

		void OnInit() override;
		void PushElement(IUIElement* element) override {}
		void OnGui(f32 dt) override;
	private:
		Callback m_onClick;
		ImVec2 m_size{0.f, 0.f};
	};
}