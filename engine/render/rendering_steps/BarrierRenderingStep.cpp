#include "BarrierRenderingStep.h"

#include <engine/profiler/Profiler.h>

using namespace engine::render;

CBarrierRenderingStep::CBarrierRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

void CBarrierRenderingStep::create(const FRenderingStepCreateInfo& create_info)
{
	m_barrier = create_info.synchronization.barrier;
}

void CBarrierRenderingStep::execute(float dt)
{
	if (m_barrier.type == BARRIER_FROM_GRAPHICS_TO_COMPUTE)
		m_pRenderContext->pushG2CBarriers(m_barrier.queue, m_barrier.targets);
	else if (m_barrier.type == BARRIER_FROM_GRAPHICS_TO_GRAPHICS) {}
	else if (m_barrier.type == BARRIER_FROM_COMPUTE_TO_COMPUTE)
		m_pRenderContext->pushC2CBarrier(m_barrier.queue);
	else if (m_barrier.type == BARRIER_FROM_COMPUTE_TO_GRAPHICS)
		m_pRenderContext->pushC2GBarrier(m_barrier.queue, m_barrier.targets);
}