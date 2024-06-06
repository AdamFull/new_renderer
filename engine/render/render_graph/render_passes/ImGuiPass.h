#pragma once

#include "BasePass.h"

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
			void _Render(const FRenderFrameInfo& frame_info) override;

		private:
			std::unique_ptr<IShaderObject> m_pShader{};
			std::unique_ptr<IVertexHandler> m_pVertexHandler{};
			FAPIHandle m_fontTexture{ INVALID_HANDLE };
		};
	}
}