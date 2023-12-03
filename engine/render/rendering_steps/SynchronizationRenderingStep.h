#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CSynchronizationRenderingStep : public IRenderingStep
		{
		public:
			CSynchronizationRenderingStep() = default;
			CSynchronizationRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CSynchronizationRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;

		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}