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

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			void Update(f32 dt) override;
		private:
			void _Render() override;

		private:
			FRHIHandle m_shader{};
		};
	}
}