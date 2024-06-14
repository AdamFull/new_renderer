#pragma once

#include <interface/EngineSystem.h>
#include <interface/render/RenderInterface.h>
#include <engine/interface/WindowInterface.h>

namespace engine
{
	class CImGuiEngineSystem : IEngineSystem
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
		u32 m_lastVertex{}, m_lastIndex{};
		f32 m_lastMouseX{}, m_lastMouseY{};

		render::FRHIHandle m_vertexHandler{};
		render::FRHIHandle m_fontTexture{ INVALID_HANDLE };

		render::IRenderInterface* m_pRenderInterface{ nullptr };
	};
}

//FRenderGraphRenderPassNodeCreateInfo create_info;
//create_info.name = "imgui_pass";
//create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eImGui, EQueueTypeFlagBits::eGraphics, "testing:imgui");
//create_info.pass_ci.AddWriteResource("present_target");
//AddRenderNode(create_info);