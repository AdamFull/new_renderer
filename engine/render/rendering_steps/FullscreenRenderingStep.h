#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CFullscreenRenderingStep : public IRenderingStep
		{
		public:
			CFullscreenRenderingStep() = default;
			CFullscreenRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CFullscreenRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;

		private:
			std::unique_ptr<IShaderObject> m_pShader{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}