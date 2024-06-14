#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CDirectionalShadowMapPass : public IRenderPass
		{
		public:
			CDirectionalShadowMapPass(IRenderInterface* render_interface);

			~CDirectionalShadowMapPass() = default;

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			bool CanRender() const override;

			void PostLoad() override;
		private:
			void _Render() override;
		protected:
			const std::unique_ptr<IShaderObject>& GetShaderByVBO(const IVertexHandler* vbo);
		private:
			utl::hash_map<u64, FRHIHandle> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}