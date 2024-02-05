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

			void init(const FComputePassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _execute(const FComputeFrameInfo& frame_info) override;
			const stl::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			stl::unordered_map<u64, stl::unique_ptr<IShaderObject>> m_shaders{};
			stl::string m_baseShaderName{};
		};
	}
}