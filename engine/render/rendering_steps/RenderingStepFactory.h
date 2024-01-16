#pragma once

#include <engine/render/rendering_steps/FullscreenRenderingStep.h>
#include <engine/render/rendering_steps/RenderScriptRenderingStep.h>
#include <engine/render/rendering_steps/SynchronizationRenderingStep.h>

namespace engine
{
	namespace render
	{
		struct RenderingStepFactory
		{
			std::unique_ptr<IRenderingStep> create(const FRenderingStepCreateInfo& create_info)
			{
				std::unique_ptr<IRenderingStep> new_step{};

				//if (create_info.type == ERenderingStepType::eSynchronization)
				//	new_step = std::make_unique<CSynchronizationRenderingStep>();
				//else if (create_info.type == ERenderingStepType::eRendering)
				//{
				//
				//}

				return new_step;
			}
		};
	}
}