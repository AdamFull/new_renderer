#pragma once

#include <interface/EngineSystem.h>
#include <interface/render/RenderInterface.h>
#include <engine/interface/WindowInterface.h>

namespace engine
{
	class CImGuiEngineSystem : public IEngineSystem
	{
	public:
		CImGuiEngineSystem(render::IRenderInterface* render_interface);
		~CImGuiEngineSystem() = default;

		void Initialize() override;

		void Update(f32 dt) override;

		void PreRender() override;
		void Render() override;

		void Destroy() override;
	protected:
		void HandleFocusChanged(i32 focused);
		void HandleCursorEnter(i32 enter);
		void HandleAxisInput(f32 x, f32 y, f32 xmax, f32 ymax);
		void HandleMouseButton(EKeyCode code, EKeyState state);
		void HandleMouseScroll(f32 x, f32 y);
		void HandleKeyInput(EKeyCode, EKeyState);
		void HandleCharInput(u32 character);
	private:
		f32 m_lastMouseX{}, m_lastMouseY{};
		render::IRenderInterface* m_pRenderInterface{ nullptr };
	};
}