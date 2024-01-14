#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CSemaphoreRenderingStep : public ISynchronizationStep
		{
		public:
			CSemaphoreRenderingStep() = default;
			CSemaphoreRenderingStep(IRenderInterface* render_interface);

			~CSemaphoreRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;
		};
	}
}