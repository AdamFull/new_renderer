#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CPostProcessPass : public IComputePass
		{
		public:
			CPostProcessPass(IRenderInterface* render_interface);

			~CPostProcessPass() = default;

			void Init(const FComputePassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
		private:
			void _Execute(const FComputeFrameInfo& frame_info) override;
		private:
			std::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}