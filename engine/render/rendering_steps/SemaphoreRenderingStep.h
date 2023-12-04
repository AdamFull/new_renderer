#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CSemaphoreRenderingStep : public IRenderingStep
		{
		public:
			CSemaphoreRenderingStep() = default;
			CSemaphoreRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CSemaphoreRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;

		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}