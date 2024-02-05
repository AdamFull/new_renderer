#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CFullscreenPass : public IRenderPass
		{
		public:
			CFullscreenPass(IRenderInterface* render_interface);

			~CFullscreenPass() = default;

			void init(const FRenderPassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _render(const FRenderFrameInfo& frame_info) override;

		private:
			stl::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}