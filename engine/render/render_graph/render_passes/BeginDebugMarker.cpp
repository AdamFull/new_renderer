#include "BeginDebugMarker.h"

using namespace engine::render;

CBeginDebugMarker::CBeginDebugMarker(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CBeginDebugMarker::Init(const std::string& name, u32 color, EQueueTypeFlagBits queue)
{
	m_markerName = name;
	m_markerColor = color;
	m_queue = queue;
}

void CBeginDebugMarker::Build(const FPassBuildInfo& build_info)
{

}

void CBeginDebugMarker::Execute(f32 dt)
{
	IBasePass::Execute(dt);

	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->BeginDebugLabel(m_markerName.c_str(), m_queue, m_markerColor);
}