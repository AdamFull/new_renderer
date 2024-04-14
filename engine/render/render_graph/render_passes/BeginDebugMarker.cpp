#include "BeginDebugMarker.h"

using namespace engine::render;

CBeginDebugMarker::CBeginDebugMarker(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CBeginDebugMarker::init(const std::string& name, u32 color, EQueueTypeFlagBits queue)
{
	m_markerName = name;
	m_markerColor = color;
	m_queue = queue;
}

void CBeginDebugMarker::build(const FPassBuildInfo& build_info)
{

}

void CBeginDebugMarker::execute(f32 dt)
{
	IBasePass::execute(dt);

	auto& render_context = m_pRenderInterface->getRenderContext();
	render_context->beginDebugLabel(m_markerName.c_str(), m_queue, m_markerColor);
}