#pragma once

#include "BasePass.h"
#include <engine/interface/WindowInterface.h>

namespace engine
{
	namespace render
	{
		class CImGuiPass : public IRenderPass
		{
		public:
			CImGuiPass(IRenderInterface* render_interface);

			~CImGuiPass();

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			void Update(f32 dt) override;
		private:
			void _Render(const FRenderFrameInfo& frame_info) override;

			void HandleFocusChanged(i32 focused);
			void HandleCursorEnter(i32 enter);
			void HandleAxisInput(f32 x, f32 y, f32 xmax, f32 ymax);
			void HandleMouseButton(EKeyCode code, EKeyState state);
			void HandleMouseScroll(f32 x, f32 y);
			void HandleKeyInput(EKeyCode, EKeyState);
			void HandleCharInput(u32 character);
		private:
			FRHIHandle m_shader{};
			FRHIHandle m_vertexHandler{};
			FRHIHandle m_fontTexture{ INVALID_HANDLE };

			u32 m_lastVertex{}, m_lastIndex{};
			f32 m_lastMouseX{}, m_lastMouseY{};
		};
	}
}