#include "EndDebugMarker.h"

using namespace engine::render;

CEndDebugMarker::CEndDebugMarker(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CEndDebugMarker::init(EQueueTypeFlagBits queue)
{
	m_queue = queue;
}

void CEndDebugMarker::build(const FPassBuildInfo& build_info)
{

}

void CEndDebugMarker::execute(f32 dt)
{
	IBasePass::execute(dt);

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->endDebugLabel(m_queue);
}