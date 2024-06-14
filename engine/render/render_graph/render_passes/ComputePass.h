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

			void Init(const FComputePassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			void Update(f32 dt) override;
		private:
			void _Execute() override;
		private:
			FRHIHandle m_shader{};
		};
	}
}