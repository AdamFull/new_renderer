#include "RenderScriptRenderingStep.h"

#include <profiler/Profiler.h>

using namespace engine::render;

CRenderScriptRenderingStep::CRenderScriptRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CRenderScriptRenderingStep::create(const FRenderingStepCreateInfo& create_info)
{

}

void CRenderScriptRenderingStep::execute(float dt)
{

}