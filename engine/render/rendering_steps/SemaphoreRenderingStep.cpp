#include "SemaphoreRenderingStep.h"

#include <engine/profiler/Profiler.h>

using namespace engine::render;

CSemaphoreRenderingStep::CSemaphoreRenderingStep(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CSemaphoreRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	auto& sync = create_info.synchronization;

}

void CSemaphoreRenderingStep::_execute(float dt)
{

}