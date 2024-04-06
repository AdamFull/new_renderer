#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CComputeHistogramPass : public IComputePass
		{
		public:
			CComputeHistogramPass(IRenderInterface* render_interface);

			~CComputeHistogramPass() = default;

			void init(const FComputePassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _execute(const FComputeFrameInfo& frame_info) override;
		private:
			stl::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}