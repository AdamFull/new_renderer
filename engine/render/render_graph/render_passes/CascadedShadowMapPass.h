#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CCascadedShadowMapPass : public IRenderPass
		{
		public:
			CCascadedShadowMapPass(IRenderInterface* render_interface);

			~CCascadedShadowMapPass() = default;

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			bool CanRender() const override;

			void PostLoad() override;
		private:
			void _Render(const FRenderFrameInfo& frame_info) override;
		protected:
			const std::unique_ptr<IShaderObject>& GetShaderByVBO(const IVertexHandler* vbo);
		private:
			utl::hash_map<u64, u64> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}