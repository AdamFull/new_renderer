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
			void postLoad() override;
		private:
			void _render(const FRenderFrameInfo& frame_info) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			utl::hash_map<u64, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}