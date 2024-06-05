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

			void Init(const FComputePassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
		private:
			void _Execute(const FComputeFrameInfo& frame_info) override;
		private:
			u64 m_shader{ INVALID_HANDLE };
		};
	}
}