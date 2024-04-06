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

			void init(const FComputePassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _execute(const FComputeFrameInfo& frame_info) override;
			void bindShaderResources(const std::unique_ptr<IShaderObject>& shader) override;
		private:
			stl::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}