#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CFullscreenPass : public IRenderPass
		{
		public:
			CFullscreenPass(IRenderInterface* render_interface, IRenderContext* render_context);

			~CFullscreenPass() = default;

			void init(const FRenderPassCreateInfo& pass_ci) override;
			void bake(std::unordered_map<size_t, FRenderGraphAttachmentOperationInfo>& resource_info) override;
		private:
			void _render(const FRenderFrameInfo& frame_info) override;

		private:
			std::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}