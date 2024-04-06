#include "ClearPass.h"

using namespace engine::render;

CClearPass::CClearPass(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CClearPass::init(const FRenderPassCreateInfo& pass_ci)
{
	IRenderPass::init(pass_ci);
}

void CClearPass::build(const FPassBuildInfo& build_info)
{
	IRenderPass::build(build_info);

#ifndef BUILD_WITHOUT_DEBUGGIND
	m_debugName = "clear";
#endif
}

void CClearPass::_render(const FRenderFrameInfo& frame_info)
{

}