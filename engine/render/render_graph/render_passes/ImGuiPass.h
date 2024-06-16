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
			CImGuiPass(IRenderInterface* render_interface);

			~CImGuiPass();

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
		private:
			void _Render() override;

			u32 m_lastVertex{}, m_lastIndex{};
			render::FRHIHandle m_vertexHandler{};
			render::FRHIHandle m_fontTexture{ INVALID_HANDLE };
		private:
			FRHIHandle m_shader{};
		};
	}
}