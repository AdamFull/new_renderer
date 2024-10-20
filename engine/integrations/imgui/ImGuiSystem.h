#pragma once

#include <interface/EngineSystem.h>
#include <interface/render/RenderInterface.h>
#include <engine/runtime/InputEvents.h>
#include <editor/EditorWindow.h>

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
		void OnAnyInput(const InputEvent& input_event);
		void HandleKeyboardInput(const KeyInputEvent& input_event);
		void HandleMouseInput(const MouseButtonInputEvent& input_event);
		void HandleTouchInput(const TouchInputEvent& input_event);
	private:
		f32 m_lastMouseX{}, m_lastMouseY{};
		utl::vector<std::unique_ptr<IEditorWindow>> m_vWindows{};
		render::IRenderInterface* m_pRenderInterface{ nullptr };
	};
}