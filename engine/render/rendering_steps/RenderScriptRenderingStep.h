#pragma once

#include <interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CRenderScriptRenderingStep : public IRenderingStep
		{
		public:
			CRenderScriptRenderingStep() = default;
			CRenderScriptRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CRenderScriptRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;

		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}