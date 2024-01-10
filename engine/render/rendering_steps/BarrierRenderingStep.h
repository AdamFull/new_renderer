#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CBarrierRenderingStep : public ISynchronizationStep
		{
		public:
			CBarrierRenderingStep() = default;
			CBarrierRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CBarrierRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;
		};
	}
}