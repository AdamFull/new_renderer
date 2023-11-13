#pragma once

#include "FullscreenRenderingStep.h"
#include "RenderScriptRenderingStep.h"
#include "SynchronizationRenderingStep.h"

namespace engine
{
	namespace render
	{
		struct RenderingStepFactory
		{
			std::unique_ptr<IRenderingStep> create(const FRenderingStepCreateInfo& create_info)
			{
				std::unique_ptr<IRenderingStep> new_step{};

				if (create_info.type == RENDERING_STEP_SYNCHRONIZATION_TYPE)
					new_step = std::make_unique<CSynchronizationRenderingStep>();
				else if (create_info.type == RENDERING_STEP_RENDERING_TYPE)
				{

				}

				return new_step;
			}
		};
	}
}