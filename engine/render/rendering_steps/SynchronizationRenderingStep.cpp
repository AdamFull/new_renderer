#include "SynchronizationRenderingStep.h"

using namespace engine::render;

CSynchronizationRenderingStep::CSynchronizationRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CSynchronizationRenderingStep::create(const FRenderingStepCreateInfo& create_info)
{

}

void CSynchronizationRenderingStep::execute(float dt)
{

}