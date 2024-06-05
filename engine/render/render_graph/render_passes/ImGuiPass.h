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
			u64 m_shader{ INVALID_HANDLE };
			u64 m_vertexHandler{ INVALID_HANDLE };
			u64 m_fontTexture{ INVALID_HANDLE };
		};
	}
}