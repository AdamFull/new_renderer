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

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			void PostLoad() override;
		private:
			void _Render(const FRenderFrameInfo& frame_info) override;
		protected:
			const std::unique_ptr<IShaderObject>& GetShaderByVBO(const IVertexHandler* vbo);
		private:
			utl::hash_map<u64, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}