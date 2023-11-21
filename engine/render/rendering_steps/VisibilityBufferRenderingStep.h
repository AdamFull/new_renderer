#pragma once

#include <interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CVisibilityBufferRenderingStep : public IRenderingStep
		{
		public:
			CVisibilityBufferRenderingStep() = default;
			CVisibilityBufferRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CVisibilityBufferRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;

		private:
			FBeginRenderingInfo m_beginRenderingInfo{};
			std::unique_ptr<IShaderObject> m_pShader{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}