#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CComputeCullingPass : public IComputePass
		{
		public:
			CComputeCullingPass(IRenderInterface* render_interface);
			~CComputeCullingPass() = default;

			void Init(const FComputePassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			void Update(f32 dt) override;
		private:
			void _Execute() override;
			const std::unique_ptr<IShaderObject>& GetShaderByVBO(const IVertexHandler* vbo);
		private:
			utl::hash_map<u64, FRHIHandle> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}