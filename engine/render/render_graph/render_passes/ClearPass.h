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

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
		private:
			void _Render() override;
		};
	}
}