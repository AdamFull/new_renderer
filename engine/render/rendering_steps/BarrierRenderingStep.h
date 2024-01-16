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
			CBarrierRenderingStep(IRenderInterface* render_interface);

			~CBarrierRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(f32 dt) override;
		};
	}
}