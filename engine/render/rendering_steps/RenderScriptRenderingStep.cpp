#include "RenderScriptRenderingStep.h"

#include <engine/profiler/Profiler.h>

using namespace engine::render;

CRenderScriptRenderingStep::CRenderScriptRenderingStep(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CRenderScriptRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{

}

void CRenderScriptRenderingStep::_execute(float dt)
{

}