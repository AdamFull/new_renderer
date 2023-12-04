#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CBarrierRenderingStep : public IRenderingStep
		{
		public:
			CBarrierRenderingStep() = default;
			CBarrierRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CBarrierRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;

		private:
			FRenderingStepBarrier m_barrier{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}