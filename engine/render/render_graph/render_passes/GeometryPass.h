#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CGeometryPass : public IRenderPass
		{
		public:
			CGeometryPass(IRenderInterface* render_interface);

			~CGeometryPass() = default;

			void init(const FRenderPassCreateInfo& pass_ci) override;
			void build(const FPassBuildInfo& build_info) override;
		private:
			void _render(const FRenderFrameInfo& frame_info) override;
		protected:
			const stl::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			stl::unordered_map<u64, stl::unique_ptr<IShaderObject>> m_shaders{};
			stl::string m_baseShaderName{};
		};
	}
}