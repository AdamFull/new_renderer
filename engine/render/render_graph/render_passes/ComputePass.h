#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CComputePass : public IComputePass
		{
		public:
			CComputePass(IRenderInterface* render_interface);

			~CComputePass() = default;

			void init(const FComputePassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _execute(const FComputeFrameInfo& frame_info) override;
		private:
			std::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}