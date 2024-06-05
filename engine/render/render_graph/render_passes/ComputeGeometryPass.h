#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CComputeGeometryPass : public IComputePass
		{
		public:
			CComputeGeometryPass(IRenderInterface* render_interface);

			~CComputeGeometryPass() = default;

			void Init(const FComputePassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
		private:
			void _Execute(const FComputeFrameInfo& frame_info) override;
			const std::unique_ptr<IShaderObject>& GetShaderByVBO(const IVertexHandler* vbo);
		private:
			utl::hash_map<u64, u64> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}