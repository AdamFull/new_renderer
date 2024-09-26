#pragma once

#include <engine/render/render_graph/render_passes/BasePass.h>

namespace engine
{
	namespace render
	{
		class CDebugDrawPass : public IRenderPass
		{
		public:
			CDebugDrawPass(IRenderInterface* render_interface);

			~CDebugDrawPass();

			void Init(const FRenderPassCreateInfo& pass_ci) override;
			void Build(const FPassBuildInfo& build_info) override;
			void Update(f32 dt) override;
		private:
			void Render2D();
			void Render3D();
			void _Render() override;
			
			FRHIHandle m_fontTexture{};

			u32 m_lastVertex{}, m_lastIndex{};
			FRHIHandle m_vertexHandler{};
			FRHIHandle m_shader{};
		};
	}
}