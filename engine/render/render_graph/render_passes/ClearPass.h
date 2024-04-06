#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CClearPass : public IRenderPass
		{
		public:
			CClearPass(IRenderInterface* render_interface);

			~CClearPass() = default;

			void init(const FRenderPassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _render(const FRenderFrameInfo& frame_info) override;
		};
	}
}