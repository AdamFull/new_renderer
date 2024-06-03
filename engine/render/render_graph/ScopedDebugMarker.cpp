#include "ScopedDebugMarker.h"

using namespace engine::render;

CScopedDebugMarker::CScopedDebugMarker(IRenderGraph* render_graph, const std::string& name, u32 color, EQueueTypeFlagBits queue)
{
	m_pRenderGraph = render_graph;
	m_srName = name;
	m_queue = queue;

	m_pRenderGraph->PushDebugMarker(name, color, queue);
}

CScopedDebugMarker::~CScopedDebugMarker()
{
	m_pRenderGraph->PopDebugMarker(m_srName, m_queue);
}