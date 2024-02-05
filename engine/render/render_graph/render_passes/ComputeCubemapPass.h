#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CComputeCubemapPass : public IComputePass
		{
		public:
			CComputeCubemapPass(IRenderInterface* render_interface);

			~CComputeCubemapPass() = default;

			void init(const FComputePassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _execute(const FComputeFrameInfo& frame_info) override;
		private:
			stl::unique_ptr<IShaderObject> m_pShader{};
		};
	}
}