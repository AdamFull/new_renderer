#pragma once

#include <engine/render/render_graph/render_passes/BasePass.h>
#include <engine/interface/WindowInterface.h>

namespace engine
{
	namespace render
	{
		struct FImDbgDrawRes
		{
			u32 lastVertex{}, lastIndex{};
			FRHIHandle vertexHandler{};
			FRHIHandle shader{};
		};

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

			FImDbgDrawRes m_2d{};
			FImDbgDrawRes m_3d{};
		};
	}
}