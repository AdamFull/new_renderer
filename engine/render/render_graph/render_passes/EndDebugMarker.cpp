#include "EndDebugMarker.h"

using namespace engine::render;

CEndDebugMarker::CEndDebugMarker(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CEndDebugMarker::Init(EQueueTypeFlagBits queue)
{
	m_queue = queue;
}

void CEndDebugMarker::Build(const FPassBuildInfo& build_info)
{

}

void CEndDebugMarker::Execute(f32 dt)
{
	IBasePass::Execute(dt);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->EndDebugLabel(m_queue);
}