#include "BarrierRenderingStep.h"

#include <engine/profiler/Profiler.h>

using namespace engine::render;

CBarrierRenderingStep::CBarrierRenderingStep(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CBarrierRenderingStep::_create(const FRenderingStepCreateInfo& create_info)
{
	
}

void CBarrierRenderingStep::_execute(f32 dt)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	if (m_barrier.type == EBarrierType::eG2C)
		render_context->pushG2CBarriers(m_barrier.queue, m_barrier.targets);
	else if (m_barrier.type == EBarrierType::eG2G) {}
	else if (m_barrier.type == EBarrierType::eC2C)
		render_context->pushC2CBarrier(m_barrier.queue, m_barrier.targets);
	else if (m_barrier.type == EBarrierType::eC2G)
		render_context->pushC2GBarrier(m_barrier.queue, m_barrier.targets);
}