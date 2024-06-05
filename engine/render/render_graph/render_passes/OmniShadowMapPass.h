#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class COmnilShadowMapPass : public IRenderPass
		{
		public:
			COmnilShadowMapPass(IRenderInterface* render_interface);

			~COmnilShadowMapPass() = default;

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			bool CanRender() const override;

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