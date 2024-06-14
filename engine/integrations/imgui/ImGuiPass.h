#pragma once

#include <engine/render/render_graph/render_passes/BasePass.h>
#include <engine/interface/WindowInterface.h>

namespace engine
{
	namespace render
	{
		class CImGuiPass : public IRenderPass
		{
		public:
			CImGuiPass(IRenderInterface* render_interface, FRHIHandle vertex_handler, FRHIHandle font_texture);

			~CImGuiPass();

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
		private:
			void _Render(const FRenderFrameInfo& frame_info) override;

			FRHIHandle m_vertexHandler{};
			FRHIHandle m_fontTexture{ INVALID_HANDLE };
		private:
			FRHIHandle m_shader{};
		};
	}
}